#include <cassert>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../Pkmn Red Save Genie/HPP Files/FileManipulation.hpp"
#include "../Pkmn Red Save Genie/HPP Files/ReadOnlyData.hpp"
#include "../Pkmn Red Save Genie/HPP Files/RedMasterJson.hpp"
#include "../Pkmn Red Save Genie/HPP Files/SaveStructure.hpp"

using namespace savegenie;

static FileManipulation::Bytes MakeSyntheticSave(std::size_t size) {
    FileManipulation::Bytes bytes(size);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<u8>((i * 37 + 0x4D) & 0xFF);
    }
    return bytes;
}

static std::string ReadTextFile(const std::string& path) {
    const FileManipulation::Bytes bytes = FileManipulation::LoadFile(path);
    return std::string(bytes.begin(), bytes.end());
}

static void WriteTextFile(const std::string& path, const std::string& text) {
    FileManipulation::WriteFile(path, FileManipulation::Bytes(text.begin(), text.end()));
}

static void ReplaceFirst(std::string* text, const std::string& needle, const std::string& replacement) {
    const std::size_t pos = text->find(needle);
    assert(pos != std::string::npos);
    text->replace(pos, needle.size(), replacement);
}

static bool Contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

static std::filesystem::path MakeTempDir() {
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() /
        ("savegenie_red_json_tests_" + std::to_string(static_cast<long long>(std::time(nullptr))));
    int suffix = 1;
    while (fs::exists(dir)) {
        dir = fs::temp_directory_path() /
            ("savegenie_red_json_tests_" + std::to_string(static_cast<long long>(std::time(nullptr))) +
             "_" + std::to_string(suffix++));
    }
    fs::create_directories(dir);
    return dir;
}

static RedMasterJsonResult ExportSyntheticSave(const std::filesystem::path& path,
                                               const FileManipulation::Bytes& bytes) {
    FileManipulation::WriteFile(path.string(), bytes);
    SaveBuffer save(bytes);
    ReadOnlyData reader(save);
    RedMasterJsonOptions options;
    options.includeGeneratedAtUtc = false;
    options.includeDecodedSummary = true;
    return RedMasterJsonExporter::ExportToFile(path.string(), save, reader, options);
}

static void TestStandardRoundTrip(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "standard.sav";
    const FileManipulation::Bytes original = MakeSyntheticSave(Gen1Layout::ExpectedSize);
    const RedMasterJsonResult exportResult = ExportSyntheticSave(savePath, original);
    assert(exportResult.ok);
    assert(exportResult.coverage.uncoveredBytes == 0);
    assert(exportResult.coverage.overlappingPrimaryBytes == 0);

    const RedMasterJsonResult reconstructResult =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, savePath.string());
    assert(reconstructResult.ok);
    assert(reconstructResult.originalSize == Gen1Layout::ExpectedSize);
    assert(reconstructResult.reconstructedSize == Gen1Layout::ExpectedSize);
    assert(reconstructResult.wholeFileSha256 == reconstructResult.reconstructedSha256);
    assert(reconstructResult.byteDifferenceCount == 0);

    const FileManipulation::Bytes rebuilt = FileManipulation::LoadFile(reconstructResult.outputPath);
    assert(rebuilt == original);
}

static void TestTrailingByteRoundTrip(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "trailing.sav";
    const FileManipulation::Bytes original = MakeSyntheticSave(Gen1Layout::ExpectedSize + 0x2C);
    const RedMasterJsonResult exportResult = ExportSyntheticSave(savePath, original);
    assert(exportResult.ok);
    assert(!exportResult.warnings.empty());

    const RedMasterJsonResult reconstructResult =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, savePath.string());
    assert(reconstructResult.ok);
    assert(reconstructResult.originalSize == Gen1Layout::ExpectedSize + 0x2C);
    assert(reconstructResult.reconstructedSize == Gen1Layout::ExpectedSize + 0x2C);
    assert(reconstructResult.byteDifferenceCount == 0);

    const FileManipulation::Bytes rebuilt = FileManipulation::LoadFile(reconstructResult.outputPath);
    assert(rebuilt == original);
}

static void TestValidationFailures(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "validation.sav";
    const FileManipulation::Bytes original = MakeSyntheticSave(Gen1Layout::ExpectedSize);
    const RedMasterJsonResult exportResult = ExportSyntheticSave(savePath, original);
    assert(exportResult.ok);

    const std::string validJson = ReadTextFile(exportResult.outputPath);

    const std::filesystem::path missingPath = dir / "missing.red.json";
    WriteTextFile(missingPath.string(), "{}");
    assert(!RedSaveReconstructor::ReconstructToFile(missingPath.string(), savePath.string()).ok);

    std::string badVersion = validJson;
    ReplaceFirst(&badVersion, "\"schemaVersion\": \"0.1.0\"", "\"schemaVersion\": \"9.9.9\"");
    const std::filesystem::path badVersionPath = dir / "bad-version.red.json";
    WriteTextFile(badVersionPath.string(), badVersion);
    assert(!RedSaveReconstructor::ReconstructToFile(badVersionPath.string(), savePath.string()).ok);

    std::string badHash = validJson;
    ReplaceFirst(&badHash,
                 "\"wholeFileSha256\": \"" + RedMasterJsonSha256Hex(original) + "\"",
                 "\"wholeFileSha256\": \"0000000000000000000000000000000000000000000000000000000000000000\"");
    const std::filesystem::path badHashPath = dir / "bad-hash.red.json";
    WriteTextFile(badHashPath.string(), badHash);
    assert(!RedSaveReconstructor::ReconstructToFile(badHashPath.string(), savePath.string()).ok);

    std::string badHex = validJson;
    const std::string marker = "\"standardSramHex\": \"";
    const std::size_t markerPos = badHex.find(marker);
    assert(markerPos != std::string::npos);
    badHex[markerPos + marker.size()] = 'g';
    const std::filesystem::path badHexPath = dir / "bad-hex.red.json";
    WriteTextFile(badHexPath.string(), badHex);
    assert(!RedSaveReconstructor::ReconstructToFile(badHexPath.string(), savePath.string()).ok);
}

static void TestCollisionSafeReconstruction(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "collision.sav";
    const FileManipulation::Bytes original = MakeSyntheticSave(Gen1Layout::ExpectedSize);
    const RedMasterJsonResult exportResult = ExportSyntheticSave(savePath, original);
    assert(exportResult.ok);

    const RedMasterJsonResult first =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, savePath.string());
    const RedMasterJsonResult second =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, savePath.string());
    assert(first.ok);
    assert(second.ok);
    assert(first.outputPath != second.outputPath);
    assert(std::filesystem::exists(first.outputPath));
    assert(std::filesystem::exists(second.outputPath));
}

static void TestDeterministicExportMode(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "deterministic.sav";
    const FileManipulation::Bytes original = MakeSyntheticSave(Gen1Layout::ExpectedSize);
    const RedMasterJsonResult first = ExportSyntheticSave(savePath, original);
    const RedMasterJsonResult second = ExportSyntheticSave(savePath, original);
    assert(first.ok);
    assert(second.ok);
    assert(ReadTextFile(first.outputPath) == ReadTextFile(second.outputPath));
}

static void TestExpandedDecodedHierarchy(const std::filesystem::path& dir) {
    const std::filesystem::path savePath = dir / "semantic.sav";
    SaveBuffer save(FileManipulation::Bytes(Gen1Layout::ExpectedSize, 0));
    auto& bytes = save.BytesMutable();

    Gen1TextCodec::EncodeName(save, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen, "MAQ");
    Gen1TextCodec::EncodeName(save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen, "BLUE");
    BcdCodec::WriteBcd3(save, Gen1Layout::MoneyOff, 123456);
    BcdCodec::WriteBcd2(save, Gen1Layout::CoinsOff, 42);
    bytes[Gen1Layout::BadgesOff] = 0x01;

    save.SetBit(Gen1Layout::PokedexOwnedOff + 1, 7, true); // Pidgey, National Dex #16.
    save.SetBit(Gen1Layout::PokedexSeenOff + 1, 7, true);

    bytes[Gen1Layout::BagItemsCountOff] = 1;
    bytes[Gen1Layout::BagItemsPairsOff] = 0x04; // POKE BALL
    bytes[Gen1Layout::BagItemsPairsOff + 1] = 7;
    bytes[Gen1Layout::PCItemBoxCountOff] = 1;
    bytes[Gen1Layout::PCItemBoxPairsOff] = 0x14; // POTION
    bytes[Gen1Layout::PCItemBoxPairsOff + 1] = 3;

    bytes[Gen1Layout::PartyCountOff] = 1;
    bytes[Gen1Layout::PartySpeciesOff] = 36; // PIDGEY
    const std::size_t mon = Gen1Layout::PartyStructsOff;
    bytes[mon + 0x00] = 36;
    bytes[mon + 0x03] = 3;
    bytes[mon + 0x04] = 0x00;
    bytes[mon + 0x08] = 16; // GUST
    bytes[mon + 0x0C] = 0x10;
    bytes[mon + 0x0D] = 0x01;
    bytes[mon + 0x10] = 0x39;
    bytes[mon + 0x1B] = 0x92;
    bytes[mon + 0x1C] = 0xAE;
    bytes[mon + 0x1D] = 0xE3;
    bytes[mon + 0x21] = 3;
    bytes[mon + 0x22] = 0x00;
    bytes[mon + 0x23] = 0x0F;
    bytes[mon + 0x24] = 0x00;
    bytes[mon + 0x25] = 0x08;
    bytes[mon + 0x26] = 0x00;
    bytes[mon + 0x27] = 0x07;
    bytes[mon + 0x28] = 0x00;
    bytes[mon + 0x29] = 0x08;
    bytes[mon + 0x2A] = 0x00;
    bytes[mon + 0x2B] = 0x07;
    Gen1TextCodec::EncodeName(save, Gen1Layout::PartyOTNamesOff, Gen1Layout::Gen1NameLen, "MARIO");
    Gen1TextCodec::EncodeName(save, Gen1Layout::PartyNicknamesOff, Gen1Layout::Gen1NameLen, "PEGGY");

    bytes[Gen1Layout::CurrentBoxCacheOff] = 0;

    const FileManipulation::Bytes original = save.BytesView();
    FileManipulation::WriteFile(savePath.string(), original);
    ReadOnlyData reader(save);
    RedMasterJsonOptions options;
    options.includeGeneratedAtUtc = false;
    options.includeDecodedSummary = true;
    const RedMasterJsonResult exportResult =
        RedMasterJsonExporter::ExportToFile(savePath.string(), save, reader, options);
    assert(exportResult.ok);

    const std::string json = ReadTextFile(exportResult.outputPath);
    assert(Contains(json, "\"schemaVersion\": \"0.1.0\""));
    assert(Contains(json, "\"decoded\""));
    assert(Contains(json, "\"conversionModel\""));
    assert(Contains(json, "\"pipeline\": [\"red_json\", \"shared_conversion_model\", \"fred_json\", \"firered_save_writer\"]"));
    assert(Contains(json, "\"trainer\""));
    assert(Contains(json, "\"rival\""));
    assert(Contains(json, "\"moneyAndCoins\""));
    assert(Contains(json, "\"pokedex\""));
    assert(Contains(json, "\"species\""));
    assert(Contains(json, "\"inventory\""));
    assert(Contains(json, "\"party\""));
    assert(Contains(json, "\"pcStorage\""));
    assert(Contains(json, "\"currentBoxCache\""));
    assert(Contains(json, "\"boxNumber\": null"));
    assert(Contains(json, "\"daycare\""));
    assert(Contains(json, "\"hallOfFame\""));
    assert(Contains(json, "\"events\""));
    assert(Contains(json, "\"trainerBattles\""));
    assert(Contains(json, "\"scripts\""));
    assert(Contains(json, "\"missableObjects\""));
    assert(Contains(json, "\"hiddenItems\""));
    assert(Contains(json, "\"hiddenCoins\""));
    assert(Contains(json, "\"visitedTowns\""));
    assert(Contains(json, "\"worldState\""));
    assert(Contains(json, "\"checksums\""));
    assert(Contains(json, "\"name\": \"PIDGEY\""));
    assert(Contains(json, "\"nickname\": { \"value\": \"PEGGY\""));
    assert(Contains(json, "\"move\": { \"name\": \"GUST\""));
    assert(Contains(json, "\"dvToIvPolicy\""));
    assert(Contains(json, "\"draft_dv_times_2_plus_1\""));
    assert(Contains(json, "\"statExperienceToEvPolicy\""));
    assert(Contains(json, "\"heldItemPolicy\""));
    assert(Contains(json, "\"classificationTable\""));
    assert(Contains(json, "\"directTransfer\""));
    assert(Contains(json, "\"semanticTranslation\""));
    assert(Contains(json, "\"redOnlyPreservation\""));
    assert(Contains(json, "\"unsupportedOrPolicyRequired\""));
    assert(Contains(json, "\"partyCountMatchesArray\": true"));
    assert(Contains(json, "\"pokedexOwnedCountMatchesList\": true"));
    assert(Contains(json, "\"semanticValidationBlocksRawReconstruction\": false"));

    const RedMasterJsonResult reconstructResult =
        RedSaveReconstructor::ReconstructToFile(exportResult.outputPath, savePath.string());
    assert(reconstructResult.ok);
    assert(reconstructResult.byteDifferenceCount == 0);
    assert(FileManipulation::LoadFile(reconstructResult.outputPath) == original);
}

int main() {
    const std::filesystem::path dir = MakeTempDir();
    TestStandardRoundTrip(dir);
    TestTrailingByteRoundTrip(dir);
    TestValidationFailures(dir);
    TestCollisionSafeReconstruction(dir);
    TestDeterministicExportMode(dir);
    TestExpandedDecodedHierarchy(dir);
    std::cout << "red_master_json_tests: PASS\n";
    return 0;
}
