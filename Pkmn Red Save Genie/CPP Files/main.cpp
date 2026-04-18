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

#include "FileManipulation.hpp"
#include "SaveStructure.hpp"
#include "ReadOnlyData.hpp"

// =========================================================
// JSON Export (PokemonBoxes.json)
// =========================================================
static std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) out += '?';
                else out += c;
                break;
        }
    }
    return out;
}

static std::string ToHexByteString(savegenie::u8 v) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex
        << std::setw(2) << std::setfill('0')
        << static_cast<int>(v)
        << std::nouppercase << std::dec;
    return oss.str();
}

static bool WritePokemonBoxesJson(const savegenie::PokemonBoxesExport& ex, const std::string& outPath) {
    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;

    f << "{\n";
    f << "  \"game\": \"Pokemon Red (Gen I)\",\n";
    f << "  \"boxes\": [\n";

    for (std::size_t bi = 0; bi < ex.boxes.size(); ++bi) {
        const auto& box = ex.boxes[bi];
        f << "    {\n";
        f << "      \"boxNumber\": " << box.boxNumber << ",\n";
        f << "      \"label\": \"" << JsonEscape(box.label) << "\",\n";
        f << "      \"pokemonCount\": " << box.pokemonCount << ",\n";
        f << "      \"pokemon\": [\n";

        for (std::size_t mi = 0; mi < box.pokemon.size(); ++mi) {
            const auto& mon = box.pokemon[mi];
            f << "        {\n";
            f << "          \"position\": " << mon.position << ",\n";
            f << "          \"speciesId\": " << static_cast<int>(mon.speciesId) << ",\n";
            f << "          \"speciesName\": \"" << JsonEscape(mon.speciesName) << "\",\n";
            f << "          \"dexNo\": \"" << JsonEscape(mon.dexNo) << "\",\n";
            f << "          \"nickname\": \"" << JsonEscape(mon.nickname) << "\",\n";
            f << "          \"otName\": \"" << JsonEscape(mon.otName) << "\",\n";
            f << "          \"otIdNo\": " << mon.otIdNo << ",\n";
            f << "          \"level\": " << static_cast<int>(mon.level) << ",\n";
            f << "          \"expPoints\": " << mon.expPoints << ",\n";

            // DVs
            f << "          \"dvs\": { "
              << "\"hp\": " << static_cast<int>(mon.dvHP) << ", "
              << "\"atk\": " << static_cast<int>(mon.dvAtk) << ", "
              << "\"def\": " << static_cast<int>(mon.dvDef) << ", "
              << "\"spd\": " << static_cast<int>(mon.dvSpd) << ", "
              << "\"spc\": " << static_cast<int>(mon.dvSpc) << " },\n";

            // Stat Exp
            f << "          \"statExp\": { "
              << "\"hp\": " << mon.statExpHP << ", "
              << "\"atk\": " << mon.statExpAtk << ", "
              << "\"def\": " << mon.statExpDef << ", "
              << "\"spd\": " << mon.statExpSpd << ", "
              << "\"spc\": " << mon.statExpSpc << " },\n";

            // Visible stats
            f << "          \"stats\": { "
              << "\"hpCurrent\": " << mon.stats.hpCurrent << ", "
              << "\"hpMax\": " << mon.stats.hpMax << ", "
              << "\"attack\": " << mon.stats.attack << ", "
              << "\"defense\": " << mon.stats.defense << ", "
              << "\"speed\": " << mon.stats.speed << ", "
              << "\"special\": " << mon.stats.special << ", "
              << "\"statusHex\": \"" << ToHexByteString(mon.stats.status) << "\" },\n";

            // Moves
            f << "          \"moves\": [\n";
            const std::size_t moveCount = std::min<std::size_t>(4, mon.moves.size());
            for (std::size_t mv = 0; mv < moveCount; ++mv) {
                const auto& m = mon.moves[mv];
                f << "            { "
                  << "\"moveId\": " << static_cast<int>(m.moveId) << ", "
                  << "\"moveName\": \"" << JsonEscape(m.moveName) << "\", "
                  << "\"ppCurrent\": " << static_cast<int>(m.ppCurrent) << ", "
                  << "\"ppMax\": " << static_cast<int>(m.ppMax)
                  << " }";
                if (mv + 1 < moveCount) f << ",";
                f << "\n";
            }
            f << "          ]\n";

            f << "        }";
            if (mi + 1 < box.pokemon.size()) f << ",";
            f << "\n";
        }

        f << "      ]\n";
        f << "    }";
        if (bi + 1 < ex.boxes.size()) f << ",";
        f << "\n";
    }

    f << "  ]\n";
    f << "}\n";

    return true;
}

// =========================================================
// JSON Export (PokemonSummary.json) — small, UI-friendly
// =========================================================
static bool WritePokemonSummaryJson(
    const savegenie::ReadOnlyData& reader,
    const std::string& inputPath,
    const std::string& backupPath,
    std::size_t saveSize,
    bool sizeLooksExpected,
    bool mainChecksumValid,
    const std::string& outPath
) {
    using namespace savegenie;

    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;

    // Party roster (compact)
    const PokemonBox party = reader.GetPartyAsBox0();

    // PC box counts (compact)
    int boxCounts[12] = {0};
    for (int i = 1; i <= 12; ++i) {
        try {
            const PokemonBox b = reader.GetPCBox(i);
            boxCounts[i - 1] = b.pokemonCount;
        } catch (...) {
            boxCounts[i - 1] = 0;
        }
    }

    f << "{\n";
    f << "  \"game\": \"Pokemon Red (Gen I)\",\n";

    f << "  \"file\": {\n";
    f << "    \"input\": \"" << JsonEscape(inputPath) << "\",\n";
    f << "    \"backup\": \"" << JsonEscape(backupPath) << "\",\n";
    f << "    \"sizeBytes\": " << static_cast<unsigned long long>(saveSize) << ",\n";
    f << "    \"sizeHex\": \"0x" << std::hex << saveSize << std::dec << "\",\n";
    f << "    \"expected32KB\": " << (sizeLooksExpected ? "true" : "false") << "\n";
    f << "  },\n";

    f << "  \"checksums\": {\n";
    f << "    \"mainValid\": " << (mainChecksumValid ? "true" : "false") << "\n";
    f << "  },\n";

    f << "  \"party\": {\n";
    f << "    \"count\": " << party.pokemonCount << ",\n";
    f << "    \"pokemon\": [\n";
    for (std::size_t i = 0; i < party.pokemon.size(); ++i) {
        const auto& mon = party.pokemon[i];
        f << "      {\n";
        f << "        \"position\": " << mon.position << ",\n";
        f << "        \"speciesId\": " << static_cast<int>(mon.speciesId) << ",\n";
        f << "        \"speciesName\": \"" << JsonEscape(mon.speciesName) << "\",\n";
        f << "        \"dexNo\": \"" << JsonEscape(mon.dexNo) << "\",\n";
        f << "        \"nickname\": \"" << JsonEscape(mon.nickname) << "\",\n";
        f << "        \"otName\": \"" << JsonEscape(mon.otName) << "\",\n";
        f << "        \"otIdNo\": " << mon.otIdNo << ",\n";
        f << "        \"level\": " << static_cast<int>(mon.level) << "\n";
        f << "      }";
        if (i + 1 < party.pokemon.size()) f << ",";
        f << "\n";
    }
    f << "    ]\n";
    f << "  },\n";

    f << "  \"pcBoxes\": {\n";
    f << "    \"counts\": [";
    for (int i = 0; i < 12; ++i) {
        f << boxCounts[i];
        if (i != 11) f << ", ";
    }
    f << "]\n";
    f << "  },\n";

    f << "  \"outputs\": {\n";
    f << "    \"boxesJson\": \"PokemonBoxes.json\",\n";
    f << "    \"summaryTxt\": \"SaveGenieSummary.txt\"\n";
    f << "  }\n";

    f << "}\n";
    return true;
}

// =========================================================
// Save Summary Export (SaveGenieSummary.txt)
// =========================================================
static bool WriteSaveGenieSummaryTxt(const std::string& text, const std::string& outPath) {
    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;
    f.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(f);
}

int main() {
    using namespace savegenie;

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
        // 1) Create backup (safe)
        const std::string backupPath = FileManipulation::BackupFile(inputPath);

        // 2) Load bytes
        FileManipulation::Bytes bytes = FileManipulation::LoadFile(inputPath);

        // 3) Wrap in SaveBuffer (safe access)
        SaveBuffer save(std::move(bytes));

        // 5) Dump readable summary (to terminal + file)
        ReadOnlyData reader(save);

        std::ostringstream summary;
        summary << "Input:  " << inputPath << "\n";
        summary << "Backup: " << backupPath << "\n";
        summary << "Size:   0x" << std::hex << save.Size() << std::dec << " bytes\n";

        if (!SaveValidator::HasExpectedSize(save)) {
            summary << "[WARN] Save size is not 0x8000 (32KB). This may not be a Gen I save.\n";
        }

        summary << "Main Checksum: "
                << (SaveValidator::HasValidMainChecksum(save) ? "VALID" : "INVALID")
                << "\n\n";

        summary << reader.DumpFullSummary() << "\n";

        // Print once to terminal
        std::cout << summary.str();

        // Also export to a text file
        const std::string summaryPath = "SaveGenieSummary.txt";
        if (WriteSaveGenieSummaryTxt(summary.str(), summaryPath)) {
            std::cout << "\n[OK] Wrote summary export: " << summaryPath << "\n";
        } else {
            std::cout << "\n[ERROR] Failed to write summary export: " << summaryPath << "\n";
        }

        // 6) Export Party (Box 0) + PC Boxes (1..12) to JSON
        const PokemonBoxesExport ex = reader.GetAllBoxesExport();
        const std::string jsonPath = "PokemonBoxes.json";
        if (WritePokemonBoxesJson(ex, jsonPath)) {
            std::cout << "\n[OK] Wrote JSON export: " << jsonPath << "\n";
        } else {
            std::cout << "\n[ERROR] Failed to write JSON export: " << jsonPath << "\n";
        }

        // 7) Export small summary JSON (UI-friendly)
        const std::string smallJsonPath = "PokemonSummary.json";
        const bool sizeOk = SaveValidator::HasExpectedSize(save);
        const bool mainOk = SaveValidator::HasValidMainChecksum(save);
        if (WritePokemonSummaryJson(reader, inputPath, backupPath, save.Size(), sizeOk, mainOk, smallJsonPath)) {
            std::cout << "\n[OK] Wrote small JSON export: " << smallJsonPath << "\n";
        } else {
            std::cout << "\n[ERROR] Failed to write small JSON export: " << smallJsonPath << "\n";
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }
}
