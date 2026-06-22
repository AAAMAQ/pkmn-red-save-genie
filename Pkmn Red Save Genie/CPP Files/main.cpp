//
//  main.cpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Reader-only test harness.
//   - Flow: Load save -> backup -> validate -> dump readable summary.
//

#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>
#include <optional>
#include <vector>

#include "FileManipulation.hpp"
#include "SaveStructure.hpp"
#include "ReadOnlyData.hpp"
#include "WriteOnlyData.hpp"
#include "RedMasterJson.hpp"
#include "RedTestExports.hpp"

// =========================================================
// Terminal Transcript Capture
// =========================================================

class TeeStreamBuf : public std::streambuf {
public:
    TeeStreamBuf(std::streambuf* terminal, std::ostream& capture)
        : terminal_(terminal), capture_(capture) {}

protected:
    int overflow(int ch) override {
        if (ch == traits_type::eof()) {
            return traits_type::not_eof(ch);
        }

        const char c = static_cast<char>(ch);
        const int terminalResult = terminal_->sputc(c);
        capture_.put(c);

        if (terminalResult == traits_type::eof() || !capture_) {
            return traits_type::eof();
        }

        return ch;
    }

    std::streamsize xsputn(const char* s, std::streamsize count) override {
        const std::streamsize terminalWritten = terminal_->sputn(s, count);
        capture_.write(s, count);

        if (terminalWritten != count || !capture_) {
            return 0;
        }

        return count;
    }

    int sync() override {
        const int terminalResult = terminal_->pubsync();
        capture_.flush();
        return (terminalResult == 0 && capture_) ? 0 : -1;
    }

private:
    std::streambuf* terminal_ = nullptr;
    std::ostream& capture_;
};

class ScopedTerminalTranscript {
public:
    ScopedTerminalTranscript()
        : coutBuf_(std::cout.rdbuf(), capture_),
          cerrBuf_(std::cerr.rdbuf(), capture_),
          oldCout_(std::cout.rdbuf(&coutBuf_)),
          oldCerr_(std::cerr.rdbuf(&cerrBuf_)) {}

    ~ScopedTerminalTranscript() {
        Restore();
    }

    ScopedTerminalTranscript(const ScopedTerminalTranscript&) = delete;
    ScopedTerminalTranscript& operator=(const ScopedTerminalTranscript&) = delete;

    void AppendInputEcho(const std::string& line) {
        capture_ << line << "\n";
    }

    std::string Text() const {
        return capture_.str();
    }

    void Restore() {
        if (!restored_) {
            std::cout.rdbuf(oldCout_);
            std::cerr.rdbuf(oldCerr_);
            restored_ = true;
        }
    }

private:
    std::ostringstream capture_;
    TeeStreamBuf coutBuf_;
    TeeStreamBuf cerrBuf_;
    std::streambuf* oldCout_ = nullptr;
    std::streambuf* oldCerr_ = nullptr;
    bool restored_ = false;
};

static ScopedTerminalTranscript* gTerminalTranscript = nullptr;

static void CaptureInputEcho(const std::string& line) {
    if (gTerminalTranscript) {
        gTerminalTranscript->AppendInputEcho(line);
    }
}

// =========================================================
// Safe Editor Helpers
// =========================================================

static constexpr int kEditorNameMaxChars = 7;
static constexpr std::size_t kUnknownOffset = static_cast<std::size_t>(-1);

enum class SaveEditChangeKind {
    Money,
    Coins,
    TrainerName,
    RivalName,
    Badges,
    BagItemQuantity,
    PCItemQuantity,
};

struct SaveEditChange {
    SaveEditChangeKind kind = SaveEditChangeKind::Money;
    std::string field;
    std::string oldValue;
    std::string newValue;
    std::string warning;
    int itemId = -1;
    int oldInt = 0;
    int newInt = 0;
    std::string newText;
    std::size_t byteOffset = kUnknownOffset;
};

static const char* kEditorBadgeNames[8] = {
    "Boulder Badge",
    "Cascade Badge",
    "Thunder Badge",
    "Rainbow Badge",
    "Soul Badge",
    "Marsh Badge",
    "Volcano Badge",
    "Earth Badge"
};

static std::string Trim(const std::string& s) {
    std::size_t first = 0;
    while (first < s.size() && std::isspace(static_cast<unsigned char>(s[first]))) {
        ++first;
    }

    std::size_t last = s.size();
    while (last > first && std::isspace(static_cast<unsigned char>(s[last - 1]))) {
        --last;
    }

    return s.substr(first, last - first);
}

static std::string LowerAscii(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

static std::string UpperAsciiForInput(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

static bool AskYesNo(const std::string& prompt) {
    while (true) {
        std::cout << prompt;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n[WARN] Input ended. Treating this as 'no'.\n";
            return false;
        }
        CaptureInputEcho(line);

        const std::string answer = LowerAscii(Trim(line));
        if (answer == "y" || answer == "yes") return true;
        if (answer == "n" || answer == "no") return false;

        std::cout << "Please answer y/yes or n/no.\n";
    }
}

static bool ParseIntegerStrict(const std::string& text, int* out) {
    try {
        std::size_t consumed = 0;
        const long long value = std::stoll(text, &consumed, 10);
        if (consumed != text.size()) return false;
        if (value < static_cast<long long>(std::numeric_limits<int>::min()) ||
            value > static_cast<long long>(std::numeric_limits<int>::max())) {
            return false;
        }
        *out = static_cast<int>(value);
        return true;
    } catch (...) {
        return false;
    }
}

static std::optional<int> ReadOptionalIntInRange(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::cout << prompt;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n[WARN] Input ended. Cancelling this edit.\n";
            return std::nullopt;
        }
        CaptureInputEcho(line);

        line = Trim(line);
        if (line.empty()) return std::nullopt;

        int value = 0;
        if (!ParseIntegerStrict(line, &value)) {
            std::cout << "Please enter a whole number.\n";
            continue;
        }

        if (value < minValue || value > maxValue) {
            std::cout << "Value must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }

        return value;
    }
}

static std::optional<int> ReadMenuChoice(int minValue, int maxValue) {
    while (true) {
        std::cout << "Choose an option: ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n[WARN] Input ended. Exiting the editor menu.\n";
            return std::nullopt;
        }
        CaptureInputEcho(line);

        line = Trim(line);

        int value = 0;
        if (!ParseIntegerStrict(line, &value)) {
            std::cout << "Please enter a menu number.\n";
            continue;
        }

        if (value < minValue || value > maxValue) {
            std::cout << "Menu option must be between " << minValue << " and " << maxValue << ".\n";
            continue;
        }

        return value;
    }
}

static bool IsSupportedEditorNameChar(char c) {
    return c == ' ' ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

static std::optional<std::string> ReadValidatedName(const std::string& prompt) {
    while (true) {
        std::cout << prompt;

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n[WARN] Input ended. Cancelling this edit.\n";
            return std::nullopt;
        }
        CaptureInputEcho(line);

        line = Trim(line);
        if (line.empty()) return std::nullopt;

        if (static_cast<int>(line.size()) > kEditorNameMaxChars) {
            std::cout << "Name is too long. This editor MVP allows up to "
                      << kEditorNameMaxChars << " visible characters.\n";
            continue;
        }

        bool supported = true;
        for (char c : line) {
            if (!IsSupportedEditorNameChar(c)) {
                supported = false;
                break;
            }
        }

        if (!supported) {
            std::cout << "Unsupported character. Supported characters: A-Z, 0-9, and space.\n";
            continue;
        }

        return UpperAsciiForInput(line);
    }
}

static std::string BadgeListString(savegenie::u8 badges) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(badges) << std::dec << " (";

    bool first = true;
    for (int i = 0; i < 8; ++i) {
        const bool owned = (badges & static_cast<savegenie::u8>(1u << i)) != 0;
        if (owned) {
            if (!first) oss << ", ";
            oss << kEditorBadgeNames[i];
            first = false;
        }
    }

    if (first) oss << "no badges";
    oss << ")";
    return oss.str();
}

static void PrintBadgeEditorState(savegenie::u8 badges) {
    for (int i = 0; i < 8; ++i) {
        const bool owned = (badges & static_cast<savegenie::u8>(1u << i)) != 0;
        std::cout << (i + 1) << ". " << kEditorBadgeNames[i]
                  << " [" << (owned ? "Owned" : "Not owned") << "]\n";
    }
}

static bool SameChangeTarget(const SaveEditChange& a, const SaveEditChange& b) {
    if (a.kind != b.kind) return false;
    if (a.kind == SaveEditChangeKind::BagItemQuantity ||
        a.kind == SaveEditChangeKind::PCItemQuantity) {
        return a.itemId == b.itemId;
    }
    return true;
}

static void UpsertChange(std::vector<SaveEditChange>& changes, SaveEditChange change) {
    auto it = std::find_if(changes.begin(), changes.end(), [&](const SaveEditChange& existing) {
        return SameChangeTarget(existing, change);
    });

    if (change.oldValue == change.newValue) {
        if (it != changes.end()) changes.erase(it);
        return;
    }

    if (it != changes.end()) {
        *it = std::move(change);
    } else {
        changes.push_back(std::move(change));
    }
}

static void PrintChangeReport(const std::vector<SaveEditChange>& changes) {
    std::cout << "\nProposed changes:\n";
    if (changes.empty()) {
        std::cout << "(none)\n";
        return;
    }

    for (const auto& change : changes) {
        std::cout << change.field << ":\n";
        std::cout << "  Old: " << change.oldValue << "\n";
        std::cout << "  New: " << change.newValue << "\n";
        if (!change.warning.empty()) {
            std::cout << "  Warning: " << change.warning << "\n";
        }
    }
}

static std::string BuildFullSummary(
    const std::string& inputPath,
    const std::string& backupPath,
    const savegenie::SaveBuffer& save,
    const savegenie::ReadOnlyData& reader
) {
    std::ostringstream summary;
    summary << "Input:  " << inputPath << "\n";
    summary << "Backup: " << backupPath << "\n";
    summary << "Size:   0x" << std::hex << save.Size() << std::dec << " bytes\n";

    if (!savegenie::SaveValidator::HasExpectedSize(save)) {
        summary << "[WARN] Save size is not 0x8000 (32KB). This may not be a Gen I save.\n";
    }

    summary << "Main Checksum: "
            << (savegenie::SaveValidator::HasValidMainChecksum(save) ? "VALID" : "INVALID")
            << "\n\n";

    summary << reader.DumpFullSummary() << "\n";
    return summary.str();
}

static void WriteStandardExports(
    const savegenie::ReadOnlyData& reader,
    const savegenie::SaveBuffer& save,
    const std::string& inputPath,
    const std::string& backupPath
) {
    using namespace savegenie;

    const PokemonBoxesExport ex = reader.GetAllBoxesExport();
    const std::string jsonPath = "PokemonBoxes.json";
    if (WritePokemonBoxesJson(ex, jsonPath)) {
        std::cout << "\n[OK] Wrote JSON export: " << jsonPath << "\n";
    } else {
        std::cout << "\n[ERROR] Failed to write JSON export: " << jsonPath << "\n";
    }

    const std::string smallJsonPath = "PokemonSummary.json";
    const bool sizeOk = SaveValidator::HasExpectedSize(save);
    const bool mainOk = SaveValidator::HasValidMainChecksum(save);
    const bool bank2Ok = Gen1Checksum::ValidateBankAll(save, 2);
    const bool bank3Ok = Gen1Checksum::ValidateBankAll(save, 3);
    if (WritePokemonSummaryJson(reader, inputPath, backupPath, save.Size(), sizeOk, mainOk, bank2Ok, bank3Ok, smallJsonPath)) {
        std::cout << "\n[OK] Wrote small JSON export: " << smallJsonPath << "\n";
    } else {
        std::cout << "\n[ERROR] Failed to write small JSON export: " << smallJsonPath << "\n";
    }
}

static bool WriteTerminalTranscriptExport(ScopedTerminalTranscript& transcript, const std::string& outPath) {
    // First write confirms the file is writable. Then print the terminal status
    // line and rewrite so SaveGenieSummary.txt includes that same visible line.
    if (!savegenie::WriteSaveGenieSummaryTxt(transcript.Text(), outPath)) {
        std::cout << "\n[ERROR] Failed to write terminal summary export: " << outPath << "\n";
        savegenie::WriteSaveGenieSummaryTxt(transcript.Text(), outPath);
        return false;
    }

    std::cout << "\n[OK] Wrote terminal summary export: " << outPath << "\n";
    return savegenie::WriteSaveGenieSummaryTxt(transcript.Text(), outPath);
}

static void PrintMasterJsonResult(const savegenie::RedMasterJsonResult& result) {
    if (!result.warnings.empty()) {
        for (const std::string& warning : result.warnings) {
            std::cout << "[WARN] " << warning << "\n";
        }
    }

    if (!result.errors.empty()) {
        for (const std::string& error : result.errors) {
            std::cout << "[ERROR] " << error << "\n";
        }
    }
}

static void WriteMasterJsonAndMaybeReconstruct(
    const std::string& inputPath,
    const savegenie::SaveBuffer& save,
    const savegenie::ReadOnlyData& reader
) {
    using namespace savegenie;

    if (!AskYesNo("\nDo you want to export the master .red.json file? (y/n): ")) {
        return;
    }

    RedMasterJsonOptions options;
    options.includeGeneratedAtUtc = true;
    options.includeDecodedSummary = true;

    const RedMasterJsonResult exportResult = RedMasterJsonExporter::ExportToFile(inputPath, save, reader, options);
    PrintMasterJsonResult(exportResult);
    if (!exportResult.ok) {
        std::cout << "[ERROR] Master .red.json export failed.\n";
        return;
    }

    std::cout << "[OK] Wrote master .red.json: " << exportResult.outputPath << "\n";
    std::cout << "Whole-file SHA-256: " << exportResult.wholeFileSha256 << "\n";
    std::cout << "Coverage uncovered bytes: " << exportResult.coverage.uncoveredBytes << "\n";
    std::cout << "Coverage overlapping primary bytes: " << exportResult.coverage.overlappingPrimaryBytes << "\n";

    if (!AskYesNo("\nDo you want to reconstruct the .red.json file into a .sav file? (y/n): ")) {
        return;
    }

    const RedMasterJsonResult reconstructResult =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, inputPath);
    PrintMasterJsonResult(reconstructResult);

    std::cout << "\n=== Red JSON Reconstruction Verification ===\n";
    std::cout << "Original size:      0x" << std::uppercase << std::hex
              << reconstructResult.originalSize << std::dec << "\n";
    std::cout << "Reconstructed size: 0x" << std::uppercase << std::hex
              << reconstructResult.reconstructedSize << std::dec << "\n";
    std::cout << "Original SHA-256:   " << reconstructResult.wholeFileSha256 << "\n";
    std::cout << "Rebuilt SHA-256:    " << reconstructResult.reconstructedSha256 << "\n";
    std::cout << "Byte differences:   " << reconstructResult.byteDifferenceCount << "\n";
    if (reconstructResult.firstDifferenceOffset != static_cast<std::size_t>(-1)) {
        std::cout << "First difference:   0x" << std::uppercase << std::hex
                  << reconstructResult.firstDifferenceOffset << std::dec << "\n";
    }
    std::cout << "Result:             "
              << (reconstructResult.ok ? "BYTE-IDENTICAL" : "FAILED")
              << "\n";

    if (reconstructResult.ok) {
        std::cout << "[OK] Wrote reconstructed save: " << reconstructResult.outputPath << "\n";
    }
}

static std::optional<std::size_t> FindItemQuantityOffset(
    const savegenie::SaveBuffer& save,
    savegenie::ItemListKind kind,
    savegenie::u8 itemId
) {
    using namespace savegenie;

    const std::size_t countOff = (kind == ItemListKind::Bag)
        ? Gen1Layout::BagItemsCountOff
        : Gen1Layout::PCItemBoxCountOff;
    const std::size_t pairsOff = (kind == ItemListKind::Bag)
        ? Gen1Layout::BagItemsPairsOff
        : Gen1Layout::PCItemBoxPairsOff;
    const int maxPairs = (kind == ItemListKind::Bag)
        ? Gen1Layout::BagItemsMaxPairs
        : Gen1Layout::PCItemBoxMaxPairs;

    const int count = std::clamp<int>(static_cast<int>(save.ReadU8(countOff)), 0, maxPairs);
    std::size_t off = pairsOff;

    for (int i = 0; i < count; ++i) {
        save.RequireRange(off, 2);
        const u8 existingItemId = save.ReadU8(off);
        if (existingItemId == 0xFF) break;
        if (existingItemId == itemId) return off + 1;
        off += 2;
    }

    return std::nullopt;
}

static bool FindItemQuantity(
    const savegenie::BagSummary& summary,
    savegenie::u8 itemId,
    savegenie::u8* quantityOut,
    std::string* labelOut
) {
    for (const auto& item : summary.items) {
        if (item.itemId == itemId) {
            if (quantityOut) *quantityOut = item.quantity;
            if (labelOut) {
                std::ostringstream label;
                label << item.itemName << " (" << item.itemHex << ")";
                *labelOut = label.str();
            }
            return true;
        }
    }

    return false;
}

static bool IsAllowedChangedOffset(std::size_t off, const std::vector<SaveEditChange>& changes) {
    using namespace savegenie;

    if (off == Gen1Layout::MainChecksumOff) return true;

    for (const auto& change : changes) {
        switch (change.kind) {
            case SaveEditChangeKind::Money:
                if (off >= Gen1Layout::MoneyOff && off < Gen1Layout::MoneyOff + Gen1Layout::MoneyLen) return true;
                break;
            case SaveEditChangeKind::Coins:
                if (off >= Gen1Layout::CoinsOff && off < Gen1Layout::CoinsOff + Gen1Layout::CoinsLen) return true;
                break;
            case SaveEditChangeKind::TrainerName:
                if (off >= Gen1Layout::TrainerNameOff && off < Gen1Layout::TrainerNameOff + Gen1Layout::TrainerNameLen) return true;
                break;
            case SaveEditChangeKind::RivalName:
                if (off >= Gen1Layout::RivalNameOff && off < Gen1Layout::RivalNameOff + Gen1Layout::RivalNameLen) return true;
                break;
            case SaveEditChangeKind::Badges:
                if (off == Gen1Layout::BadgesOff || off == Gen1Layout::BadgesMirrorOff) return true;
                break;
            case SaveEditChangeKind::BagItemQuantity:
            case SaveEditChangeKind::PCItemQuantity:
                if (change.byteOffset != kUnknownOffset && off == change.byteOffset) return true;
                break;
        }
    }

    return false;
}

static bool PrintChangedOffsetReport(
    const savegenie::SaveBuffer& original,
    const savegenie::SaveBuffer& edited,
    const std::vector<SaveEditChange>& changes
) {
    const auto& a = original.BytesView();
    const auto& b = edited.BytesView();
    const std::size_t minSize = std::min(a.size(), b.size());

    std::vector<std::size_t> changed;
    for (std::size_t i = 0; i < minSize; ++i) {
        if (a[i] != b[i]) changed.push_back(i);
    }

    bool offsetsAllowed = (a.size() == b.size());
    for (std::size_t off : changed) {
        if (!IsAllowedChangedOffset(off, changes)) {
            offsetsAllowed = false;
            break;
        }
    }

    std::cout << "\nChanged offsets (" << changed.size() << "):\n";
    if (changed.empty()) {
        std::cout << "(none)\n";
    } else {
        const std::size_t limit = std::min<std::size_t>(changed.size(), 80);
        for (std::size_t i = 0; i < limit; ++i) {
            std::cout << "0x" << std::uppercase << std::hex << std::setw(4)
                      << std::setfill('0') << changed[i] << std::dec << "\n";
        }
        if (changed.size() > limit) {
            std::cout << "... " << (changed.size() - limit) << " more offsets not shown\n";
        }
    }

    if (a.size() != b.size()) {
        std::cout << "[ERROR] Edited buffer size differs from the original.\n";
    }

    std::cout << "Changed-offset verification: "
              << (offsetsAllowed ? "PASS" : "FAIL")
              << " (only supported field bytes and the main checksum should change)\n";
    return offsetsAllowed;
}

static bool VerifyEditedOutput(
    const savegenie::SaveBuffer& edited,
    const std::vector<SaveEditChange>& changes
) {
    using namespace savegenie;

    ReadOnlyData reader(edited);
    const TrainerSummary trainer = reader.GetTrainerSummary();
    const BagSummary bag = reader.GetBagSummary(true);
    const BagSummary pc = reader.GetPCItemBoxSummary(true);

    bool ok = true;
    for (const auto& change : changes) {
        switch (change.kind) {
            case SaveEditChangeKind::Money:
                if (static_cast<int>(trainer.money) != change.newInt) {
                    std::cout << "[ERROR] Money verification failed. Decoded "
                              << trainer.money << ", expected " << change.newInt << ".\n";
                    ok = false;
                }
                break;
            case SaveEditChangeKind::Coins:
                if (static_cast<int>(trainer.coins) != change.newInt) {
                    std::cout << "[ERROR] Coins verification failed. Decoded "
                              << trainer.coins << ", expected " << change.newInt << ".\n";
                    ok = false;
                }
                break;
            case SaveEditChangeKind::TrainerName:
                if (trainer.trainerName != change.newText) {
                    std::cout << "[ERROR] Trainer name verification failed. Decoded "
                              << trainer.trainerName << ", expected " << change.newText << ".\n";
                    ok = false;
                }
                break;
            case SaveEditChangeKind::RivalName:
                if (trainer.rivalName != change.newText) {
                    std::cout << "[ERROR] Rival name verification failed. Decoded "
                              << trainer.rivalName << ", expected " << change.newText << ".\n";
                    ok = false;
                }
                break;
            case SaveEditChangeKind::Badges:
                if (static_cast<int>(trainer.badges) != change.newInt) {
                    std::cout << "[ERROR] Badge verification failed. Decoded "
                              << static_cast<int>(trainer.badges)
                              << ", expected " << change.newInt << ".\n";
                    ok = false;
                }
                break;
            case SaveEditChangeKind::BagItemQuantity:
            case SaveEditChangeKind::PCItemQuantity: {
                const BagSummary& list = (change.kind == SaveEditChangeKind::BagItemQuantity) ? bag : pc;
                u8 decodedQuantity = 0;
                if (!FindItemQuantity(list, static_cast<u8>(change.itemId), &decodedQuantity, nullptr)) {
                    std::cout << "[ERROR] Item verification failed. Item ID "
                              << change.itemId << " was not found after reload.\n";
                    ok = false;
                } else if (static_cast<int>(decodedQuantity) != change.newInt) {
                    std::cout << "[ERROR] Item quantity verification failed. Decoded "
                              << static_cast<int>(decodedQuantity)
                              << ", expected " << change.newInt << ".\n";
                    ok = false;
                }
                break;
            }
        }
    }

    return ok;
}

static bool WriteEditedSaveAndVerify(
    const std::string& inputPath,
    const savegenie::SaveBuffer& original,
    savegenie::SaveBuffer& working,
    const std::vector<SaveEditChange>& changes,
    std::string* editedPathOut
) {
    using namespace savegenie;

    WriteOnlyData writer(working);
    EditLog log;
    const EditMessage fix = writer.FixChecksums(&log);
    if (!fix.Ok()) {
        std::cout << "[ERROR] Could not repair main checksum: " << fix.message << "\n";
        return false;
    }

    const bool offsetsOk = PrintChangedOffsetReport(original, working, changes);
    const std::string editedPath = FileManipulation::MakeEditedPathCollisionSafe(inputPath);

    FileManipulation::WriteFile(editedPath, working.BytesView());
    std::cout << "\n[OK] Wrote edited save: " << editedPath << "\n";

    const FileManipulation::Bytes editedBytes = FileManipulation::LoadFile(editedPath);
    SaveBuffer reloaded(editedBytes);

    const bool sizeExact = SaveValidator::HasExpectedSize(reloaded);
    const bool mainOk = SaveValidator::HasValidMainChecksum(reloaded);
    const bool valuesOk = VerifyEditedOutput(reloaded, changes);

    std::cout << "Reload: OK\n";
    std::cout << "Size validation: "
              << (sizeExact ? "exact 0x8000" : "warning: not exactly 0x8000") << "\n";
    std::cout << "Main checksum after edit: " << (mainOk ? "VALID" : "INVALID") << "\n";
    std::cout << "Requested value verification: " << (valuesOk ? "PASS" : "FAIL") << "\n";

    if (editedPathOut) *editedPathOut = editedPath;

    if (!sizeExact) {
        std::cout << "[WARN] Size mismatch is reported but not treated as fatal here because "
                     "some emulator saves include trailing bytes.\n";
    }

    if (!offsetsOk || !mainOk || !valuesOk) {
        std::cout << "[ERROR] Edited save verification failed. Original save remains untouched.\n";
        return false;
    }

    std::cout << "[OK] Edited save verified. Original save remains untouched.\n";
    return true;
}

static void PrintSupportedEditFields() {
    std::cout << "\nSupported safe edits in this MVP:\n";
    std::cout << "1. Money\n";
    std::cout << "2. Coins\n";
    std::cout << "3. Trainer name\n";
    std::cout << "4. Rival name\n";
    std::cout << "5. Badge bit field\n";
    std::cout << "6. Existing bag item quantities only\n";
    std::cout << "7. Existing PC item quantities only\n";
    std::cout << "\nRead-only for now: Pokemon, event flags, location, Pokedex bits, Hall of Fame,\n";
    std::cout << "trainer ID, raw offsets, checksums, and PC box Pokemon data.\n";
    std::cout << "Supported name characters: A-Z, 0-9, and space. Lowercase input is uppercased.\n";
}

static void PrintSafeEditorMenu() {
    std::cout << "\nSafe Save Editor\n";
    std::cout << "1. Change money\n";
    std::cout << "2. Change coins\n";
    std::cout << "3. Change trainer name\n";
    std::cout << "4. Change rival name\n";
    std::cout << "5. Edit badges\n";
    std::cout << "6. Change existing bag item quantity\n";
    std::cout << "7. Change existing PC item quantity\n";
    std::cout << "8. Review pending changes\n";
    std::cout << "9. Discard pending changes\n";
    std::cout << "0. Finish editing\n";
}

static void PrintItemListForEditing(const savegenie::BagSummary& summary) {
    if (summary.items.empty()) {
        std::cout << "(no items available to edit)\n";
        return;
    }

    for (std::size_t i = 0; i < summary.items.size(); ++i) {
        const auto& item = summary.items[i];
        std::cout << (i + 1) << ". "
                  << "ID " << static_cast<int>(item.itemId)
                  << " " << item.itemHex
                  << " " << item.itemName
                  << " quantity " << static_cast<int>(item.quantity)
                  << "\n";
    }
}

static bool RunSafeEditor(
    const savegenie::SaveBuffer& original,
    const std::string& inputPath,
    std::string* editedPathOut
) {
    using namespace savegenie;

    SaveBuffer working(original.BytesView());
    WriteOnlyData editor(working);
    ReadOnlyData originalReader(original);
    const TrainerSummary originalTrainer = originalReader.GetTrainerSummary();
    const BagSummary originalBag = originalReader.GetBagSummary(true);
    const BagSummary originalPcItems = originalReader.GetPCItemBoxSummary(true);

    std::vector<SaveEditChange> changes;

    while (true) {
        PrintSafeEditorMenu();
        const std::optional<int> choice = ReadMenuChoice(0, 9);
        if (!choice.has_value()) {
            std::cout << "No edited file written.\n";
            return false;
        }

        if (*choice == 0) {
            if (changes.empty()) {
                std::cout << "No pending changes. No edited save file will be written.\n";
                return false;
            }

            PrintChangeReport(changes);
            if (!AskYesNo("\nWrite these changes to a new edited save file? (y/n): ")) {
                std::cout << "No edited file written. Pending changes remain only in memory.\n";
                if (AskYesNo("Return to the editor menu? (y/n): ")) {
                    continue;
                }
                return false;
            }

            return WriteEditedSaveAndVerify(inputPath, original, working, changes, editedPathOut);
        }

        if (*choice == 8) {
            PrintChangeReport(changes);
            continue;
        }

        if (*choice == 9) {
            working = SaveBuffer(original.BytesView());
            changes.clear();
            std::cout << "Pending changes discarded. Working copy restored from the original save.\n";
            if (!AskYesNo("Continue editing? (y/n): ")) {
                std::cout << "No edited file written.\n";
                return false;
            }
            continue;
        }

        ReadOnlyData currentReader(working);
        const TrainerSummary currentTrainer = currentReader.GetTrainerSummary();

        if (*choice == 1) {
            std::cout << "Current pending money: " << currentTrainer.money << "\n";
            const std::optional<int> value = ReadOptionalIntInRange(
                "Enter new money (0-999999), or blank to cancel: ", 0, 999999);
            if (!value.has_value()) continue;

            const EditMessage m = editor.SetMoney(static_cast<u32>(*value));
            if (!m.Ok()) {
                std::cout << "[ERROR] " << m.message << "\n";
                continue;
            }

            SaveEditChange change;
            change.kind = SaveEditChangeKind::Money;
            change.field = "Money";
            change.oldValue = std::to_string(originalTrainer.money);
            change.newValue = std::to_string(*value);
            change.oldInt = static_cast<int>(originalTrainer.money);
            change.newInt = *value;
            UpsertChange(changes, std::move(change));
            std::cout << "[OK] Pending money updated.\n";
            continue;
        }

        if (*choice == 2) {
            std::cout << "Current pending coins: " << currentTrainer.coins << "\n";
            const std::optional<int> value = ReadOptionalIntInRange(
                "Enter new coins (0-9999), or blank to cancel: ", 0, 9999);
            if (!value.has_value()) continue;

            const EditMessage m = editor.SetCoins(static_cast<u16>(*value));
            if (!m.Ok()) {
                std::cout << "[ERROR] " << m.message << "\n";
                continue;
            }

            SaveEditChange change;
            change.kind = SaveEditChangeKind::Coins;
            change.field = "Coins";
            change.oldValue = std::to_string(originalTrainer.coins);
            change.newValue = std::to_string(*value);
            change.oldInt = static_cast<int>(originalTrainer.coins);
            change.newInt = *value;
            UpsertChange(changes, std::move(change));
            std::cout << "[OK] Pending coins updated.\n";
            continue;
        }

        if (*choice == 3) {
            std::cout << "Current pending trainer name: " << currentTrainer.trainerName << "\n";
            std::cout << "Supported characters: A-Z, 0-9, space. Blank cancels.\n";
            const std::optional<std::string> value = ReadValidatedName(
                "Enter new trainer name: ");
            if (!value.has_value()) continue;

            const EditMessage m = editor.SetTrainerName(*value);
            if (!m.Ok()) {
                std::cout << "[ERROR] " << m.message << "\n";
                continue;
            }

            SaveEditChange change;
            change.kind = SaveEditChangeKind::TrainerName;
            change.field = "Trainer name";
            change.oldValue = originalTrainer.trainerName;
            change.newValue = *value;
            change.newText = *value;
            UpsertChange(changes, std::move(change));
            std::cout << "[OK] Pending trainer name updated.\n";
            continue;
        }

        if (*choice == 4) {
            std::cout << "Current pending rival name: " << currentTrainer.rivalName << "\n";
            std::cout << "Supported characters: A-Z, 0-9, space. Blank cancels.\n";
            const std::optional<std::string> value = ReadValidatedName(
                "Enter new rival name: ");
            if (!value.has_value()) continue;

            const EditMessage m = editor.SetRivalName(*value);
            if (!m.Ok()) {
                std::cout << "[ERROR] " << m.message << "\n";
                continue;
            }

            SaveEditChange change;
            change.kind = SaveEditChangeKind::RivalName;
            change.field = "Rival name";
            change.oldValue = originalTrainer.rivalName;
            change.newValue = *value;
            change.newText = *value;
            UpsertChange(changes, std::move(change));
            std::cout << "[OK] Pending rival name updated.\n";
            continue;
        }

        if (*choice == 5) {
            std::cout << "\nWarning: Badge changes may not update every related story or gym event flag.\n";
            std::cout << "Only the badge bit field will be changed. Related story flags will not be modified.\n";

            while (true) {
                const TrainerSummary badgeState = ReadOnlyData(working).GetTrainerSummary();
                std::cout << "\nCurrent pending badges:\n";
                PrintBadgeEditorState(badgeState.badges);

                const std::optional<int> badgeChoice = ReadOptionalIntInRange(
                    "Toggle badge number (1-8), enter 0 when done, or blank to cancel: ", 0, 8);
                if (!badgeChoice.has_value() || *badgeChoice == 0) break;

                const int bit = *badgeChoice - 1;
                const u8 newBadges = static_cast<u8>(badgeState.badges ^ static_cast<u8>(1u << bit));
                const EditMessage m = editor.SetBadges(newBadges);
                if (!m.Ok()) {
                    std::cout << "[ERROR] " << m.message << "\n";
                    continue;
                }

                SaveEditChange change;
                change.kind = SaveEditChangeKind::Badges;
                change.field = "Badges";
                change.oldValue = BadgeListString(originalTrainer.badges);
                change.newValue = BadgeListString(newBadges);
                change.oldInt = static_cast<int>(originalTrainer.badges);
                change.newInt = static_cast<int>(newBadges);
                change.warning = "Only the badge bit field will be changed. Related story flags will not be modified.";
                UpsertChange(changes, std::move(change));
                std::cout << "[OK] Pending badge bit field updated.\n";
            }
            continue;
        }

        if (*choice == 6 || *choice == 7) {
            const bool editingBag = (*choice == 6);
            const ItemListKind kind = editingBag ? ItemListKind::Bag : ItemListKind::PCItemBox;
            const BagSummary currentItems = editingBag
                ? ReadOnlyData(working).GetBagSummary(true)
                : ReadOnlyData(working).GetPCItemBoxSummary(true);

            std::cout << "\nExisting " << (editingBag ? "bag" : "PC item box") << " items:\n";
            PrintItemListForEditing(currentItems);
            if (currentItems.items.empty()) continue;

            const std::optional<int> slot = ReadOptionalIntInRange(
                "Select item slot, or blank to cancel: ", 1, static_cast<int>(currentItems.items.size()));
            if (!slot.has_value()) continue;

            const auto& selected = currentItems.items[static_cast<std::size_t>(*slot - 1)];
            const std::optional<int> quantity = ReadOptionalIntInRange(
                "Enter new quantity (1-99), or blank to cancel: ", 1, 99);
            if (!quantity.has_value()) continue;

            const std::optional<std::size_t> quantityOff = FindItemQuantityOffset(working, kind, selected.itemId);
            EditLog itemLog;
            const EditMessage m = editor.SetItemQuantity(kind, selected.itemId, static_cast<u8>(*quantity), &itemLog);
            if (!m.Ok()) {
                std::cout << "[ERROR] " << m.message << "\n";
                continue;
            }

            const BagSummary& originalItems = editingBag ? originalBag : originalPcItems;
            u8 originalQuantity = selected.quantity;
            std::string label = selected.itemName + " (" + selected.itemHex + ")";
            FindItemQuantity(originalItems, selected.itemId, &originalQuantity, &label);

            SaveEditChange change;
            change.kind = editingBag ? SaveEditChangeKind::BagItemQuantity : SaveEditChangeKind::PCItemQuantity;
            change.field = std::string(editingBag ? "Bag item quantity: " : "PC item quantity: ") + label;
            change.oldValue = std::to_string(static_cast<int>(originalQuantity));
            change.newValue = std::to_string(*quantity);
            change.itemId = static_cast<int>(selected.itemId);
            change.oldInt = static_cast<int>(originalQuantity);
            change.newInt = *quantity;
            change.byteOffset = quantityOff.value_or(kUnknownOffset);
            UpsertChange(changes, std::move(change));
            std::cout << "[OK] Pending item quantity updated.\n";
            continue;
        }
    }
}

int main() {
    using namespace savegenie;

    ScopedTerminalTranscript terminalTranscript;
    gTerminalTranscript = &terminalTranscript;
    int exitCode = 0;
    bool writeLegacyTranscript = false;

    // ------------------------------------------------------------
    // NOTE:
    // Replace the filename below with your own legally obtained
    // Pokémon Red save file (.sav).
    //
    // The file must be located in the same directory as the
    // executable, or provide a full path.
    //
    // This project does NOT distribute ROMs or save files.
    // ------------------------------------------------------------
    const std::string inputPath =
        "Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav";

    try {
        // 1) Optional backup. The original is never overwritten, but backups are
        // still strongly recommended before any save-research workflow.
        std::string backupPath = "(not created)";
        if (AskYesNo("Do you want to create a backup copy of the save file? (y/n): ")) {
            backupPath = FileManipulation::BackupFile(inputPath);
            std::cout << "[OK] Backup path: " << backupPath << "\n";
        } else {
            std::cout << "[WARN] Backup was skipped. This tool will still avoid overwriting the original save.\n";
        }

        // 2) Load bytes
        FileManipulation::Bytes bytes = FileManipulation::LoadFile(inputPath);

        // 3) Wrap in SaveBuffer (safe access)
        SaveBuffer save(std::move(bytes));

        // 4) Build the read-only summary from the original save.
        ReadOnlyData reader(save);
        const std::string summaryText = BuildFullSummary(inputPath, backupPath, save, reader);
        const bool mainChecksumValid = SaveValidator::HasValidMainChecksum(save);

        std::cout << summaryText;

        if (!mainChecksumValid) {
            std::cout << "\n[WARN] Editing is disabled because the main checksum is invalid.\n";
        } else if (AskYesNo("Do you want to edit your save file? (y/n): ")) {
            std::cout << "\nSafe editor mode selected.\n";
            std::cout << "The original save will never be overwritten. Edits are applied only to an in-memory copy\n";
            std::cout << "and written to a separate collision-safe '(EDITED)' save file after final confirmation.\n";
            PrintSupportedEditFields();

            if (AskYesNo("\nDo you want to change any of the supported fields? (y/n): ")) {
                std::string editedPath;
                RunSafeEditor(save, inputPath, &editedPath);
            } else {
                std::cout << "No save bytes were modified.\n";
            }
        }

        if (AskYesNo("\nDo you want to export legacy/test outputs? (y/n): ")) {
            WriteStandardExports(reader, save, inputPath, backupPath);
            writeLegacyTranscript = true;
        }

        WriteMasterJsonAndMaybeReconstruct(inputPath, save, reader);

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        exitCode = 1;
    }

    if (writeLegacyTranscript) {
        WriteTerminalTranscriptExport(terminalTranscript, "SaveGenieSummary.txt");
    }
    gTerminalTranscript = nullptr;
    terminalTranscript.Restore();

    return exitCode;
}
