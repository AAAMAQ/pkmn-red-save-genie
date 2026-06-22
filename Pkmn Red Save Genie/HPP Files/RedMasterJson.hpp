//
//  RedMasterJson.hpp
//  Pkmn Red Save Genie
//
//  Canonical, lossless, draft .red.json export/import/reconstruction support.
//

#ifndef RedMasterJson_hpp
#define RedMasterJson_hpp

#include <cstddef>
#include <string>
#include <vector>

#include "FileManipulation.hpp"
#include "ReadOnlyData.hpp"
#include "SaveStructure.hpp"

namespace savegenie {

struct RedMasterJsonOptions {
    bool includeGeneratedAtUtc = true;
    bool includeDecodedSummary = true;
};

struct RedCoverageRange {
    std::size_t start = 0;
    std::size_t endInclusive = 0;
    std::string name;
    std::string classification;
    bool decoded = false;
    std::string reconstructionPolicy = "preserve-original-bytes";
    std::string notes;
};

struct RedCoverageSummary {
    std::size_t totalStandardSramBytes = 0;
    std::size_t decodedBytes = 0;
    std::size_t partiallyDecodedBytes = 0;
    std::size_t knownRawBytes = 0;
    std::size_t runtimeBytes = 0;
    std::size_t unknownBytes = 0;
    std::size_t rawPreservedBytes = 0;
    std::size_t uncoveredBytes = 0;
    std::size_t overlappingPrimaryBytes = 0;
    std::vector<RedCoverageRange> ranges;
};

struct RedMasterJsonResult {
    bool ok = false;
    std::string outputPath;
    std::string jsonPath;
    std::string originalPath;
    std::string originalFileName;
    std::size_t originalSize = 0;
    std::size_t reconstructedSize = 0;
    std::size_t byteDifferenceCount = 0;
    std::size_t firstDifferenceOffset = static_cast<std::size_t>(-1);
    std::string wholeFileSha256;
    std::string reconstructedSha256;
    RedCoverageSummary coverage;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

struct RedMasterJsonDocument {
    FileManipulation::Bytes bytes;
    std::string originalFileName;
    std::size_t declaredTotalLength = 0;
    std::size_t declaredStandardSramLength = 0;
    std::size_t declaredTrailingLength = 0;
    std::string wholeFileSha256;
    std::string standardSramSha256;
    std::string trailingDataSha256;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

std::string RedMasterJsonSha256Hex(const FileManipulation::Bytes& bytes);
std::string RedBytesToCanonicalHex(const FileManipulation::Bytes& bytes);
bool RedCanonicalHexToBytes(const std::string& hex, FileManipulation::Bytes* out, std::string* error);
std::size_t RedCountByteDifferences(
    const FileManipulation::Bytes& a,
    const FileManipulation::Bytes& b,
    std::size_t* firstDifferenceOffset = nullptr
);

class RedCoverageReporter {
public:
    static RedCoverageSummary BuildPrimaryCoverage();
    static bool ValidatePrimaryCoverage(RedCoverageSummary* summary, std::vector<std::string>* errors);
};

class RedMasterJsonExporter {
public:
    static std::string MakeRedJsonPathCollisionSafe(const std::string& savePath);

    static RedMasterJsonResult ExportToFile(
        const std::string& inputPath,
        const SaveBuffer& save,
        const ReadOnlyData& reader,
        const RedMasterJsonOptions& options = RedMasterJsonOptions()
    );
};

class RedMasterJsonImporter {
public:
    static RedMasterJsonDocument LoadAndValidate(const std::string& jsonPath);
};

class RedSaveReconstructor {
public:
    static std::string MakeReconstructedPathCollisionSafe(const std::string& originalSavePath);

    static RedMasterJsonResult ReconstructToFile(
        const std::string& jsonPath,
        const std::string& originalSavePath = std::string()
    );
};

} // namespace savegenie

#endif /* RedMasterJson_hpp */
