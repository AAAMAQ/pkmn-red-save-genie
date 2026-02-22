//
//  ReadOnlyData.hpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - High-level, human-readable extraction layer.
//   - Converts raw save bytes (via SaveStructure) into plain English summaries.
//   - Contains NO write/edit logic.
//
//  Owns:
//   - Trainer summary extraction
//   - Location summary extraction
//   - Money / Coins display
//   - Badge interpretation
//   - Playtime formatting
//   - Box statistics (count / average level)
//   - Basic flag summaries
//
//  Does NOT:
//   - Modify save data
//   - Perform file I/O
//   - Recalculate checksums
//

#ifndef ReadOnlyData_hpp
#define ReadOnlyData_hpp

#include <string>
#include <vector>
#include <cstdint>

#include "SaveStructure.hpp"

namespace savegenie {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

// =========================================================
// Trainer Summary Model
// =========================================================

class TrainerSummary {
public:
    std::string trainerName;
    std::string rivalName;

    u16 trainerId = 0;

    u32 money = 0;
    u16 coins = 0;

    u8 badges = 0;

    u8 mapId = 0;
    u8 x = 0;
    u8 y = 0;

    u8 playHours = 0;
    u8 playMinutes = 0;
    u8 playSeconds = 0;

    std::string ToString() const;
};

// =========================================================
// Box Statistics Model
// =========================================================

class BoxStats {
public:
    int boxIndex = 0;           // 1..12
    int pokemonCount = 0;
    double averageLevel = 0.0;

    std::string ToString() const;
};

// =========================================================
// Flag Summary Model
// =========================================================

class FlagSummary {
public:
    int totalFlagsChecked = 0;
    int totalFlagsSet = 0;

    std::vector<int> setFlagIndices;

    std::string ToString() const;
};

// =========================================================
// ReadOnlyData (Main Reader Class)
// =========================================================

class ReadOnlyData {
public:
    explicit ReadOnlyData(const SaveBuffer& buffer);

    // --- Core Data ---
    TrainerSummary GetTrainerSummary() const;

    // --- PC Box Statistics ---
    BoxStats GetBoxStats(int boxIndex1to12) const;

    // --- Flags ---
    FlagSummary GetEventFlagSummary() const;

    // --- Raw Dump ---
    std::string DumpFullSummary() const;

private:
    const SaveBuffer& buffer_;

    // Internal helpers
    int CountBits(u8 byte) const;
};

} // namespace savegenie

#endif /* ReadOnlyData_hpp */
