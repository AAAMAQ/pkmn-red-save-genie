//
//  RedTestExports.hpp
//  Pkmn Red Save Genie
//
//  Legacy/test-oriented exports. These files are useful for inspection and
//  regression work, but they are not the canonical lossless .red.json format.
//

#ifndef RedTestExports_hpp
#define RedTestExports_hpp

#include <cstddef>
#include <string>
#include <string_view>

#include "ReadOnlyData.hpp"
#include "SaveStructure.hpp"

namespace savegenie {

std::string JsonEscape(std::string_view s);

bool WritePokemonBoxesJson(const PokemonBoxesExport& ex, const std::string& outPath);

bool WritePokemonSummaryJson(
    const ReadOnlyData& reader,
    const std::string& inputPath,
    const std::string& backupPath,
    std::size_t saveSize,
    bool sizeLooksExpected,
    bool mainChecksumValid,
    bool bank2AllValid,
    bool bank3AllValid,
    const std::string& outPath
);

bool WriteSaveGenieSummaryTxt(const std::string& text, const std::string& outPath);

} // namespace savegenie

#endif /* RedTestExports_hpp */
