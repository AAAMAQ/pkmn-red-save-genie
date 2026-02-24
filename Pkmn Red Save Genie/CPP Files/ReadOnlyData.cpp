//
//  ReadOnlyData.cpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Implementation of the read-only translation layer.
//   - Converts known Gen I save fields into plain English.
//   - No file I/O and no modifications.
//

#include "ReadOnlyData.hpp"
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

namespace savegenie {

// =========================================================
// TrainerSummary
// =========================================================

std::string TrainerSummary::ToString() const {
    std::ostringstream oss;
    oss << "Trainer Name: " << trainerName << "\n";
    oss << "Rival Name:   " << rivalName << "\n";

    oss << "Trainer ID:   " << trainerId << "\n";
    oss << "Money:        " << "₽"<< money << "\n";
    oss << "Coins:        " << coins << "\n";

    // Badges (bitfield) each bit represents each gym
    static const char* kBadgeNames[8] = {
        "Boulder (Brock)",
        "Cascade (Misty)",
        "Thunder (Lt. Surge)",
        "Rainbow (Erika)",
        "Soul (Koga)",
        "Marsh (Sabrina)",
        "Volcano (Blaine)",
        "Earth (Giovanni)"
    };

    /*oss << "Badges:       0x" << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(badges) << std::dec << "\n";*/

    oss << "Badges List:  "<<std::endl;
    oss << "1.";
    bool first = true;
    for (int i = 0; i < 8; ++i) {
        const bool has = (badges & static_cast<u8>(1u << i)) != 0;
        if (!first) oss << (i+1)<<".";
        first = false;
        oss << kBadgeNames[i] << (has ? " ->Yes" : " ->No")<<std::endl;
    }
    oss << "\n";

    // Map Location
    oss << "Location:     MapID=" <<static_cast<int>(mapId)<<", Hex= ("<<Gen1MapLookup::MapIDHex[static_cast<int>(mapId)]<<") "<< Gen1MapLookup::MapIDName[static_cast<int>(mapId)]
        << " X=" << static_cast<int>(x)
        << " Y=" << static_cast<int>(y) << "\n";

    oss << "Playtime:     "
        << static_cast<int>(playHours) << "h "
        << static_cast<int>(playMinutes) << "m "
        << static_cast<int>(playSeconds) << "s\n";

    return oss.str();
}

// =========================================================
// BoxStats
// =========================================================

std::string BoxStats::ToString() const {
    std::ostringstream oss;
    oss << "Box " << boxIndex << ": "
        << pokemonCount << " Pokémon";

    if (pokemonCount > 0) {
        oss << ", Avg Lv " << std::fixed << std::setprecision(2) << averageLevel;
    }

    return oss.str();
}

// =========================================================
// FlagSummary
// =========================================================

std::string FlagSummary::ToString() const {
    std::ostringstream oss;
    oss << "Flags Checked: " << totalFlagsChecked << "\n";
    oss << "Flags Set:     " << totalFlagsSet << "\n";

    if (!setFlagIndices.empty()) {
        oss << "Set Flag Indices (first 50): ";
        const std::size_t limit = std::min<std::size_t>(50, setFlagIndices.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << setFlagIndices[i];
            if (i + 1 < limit) oss << ", ";
        }
        if (setFlagIndices.size() > limit) oss << " ...";
        oss << "\n";
    }

    return oss.str();
}

// =========================================================
// ReadOnlyData
// =========================================================

ReadOnlyData::ReadOnlyData(const SaveBuffer& buffer)
: buffer_(buffer) {}

TrainerSummary ReadOnlyData::GetTrainerSummary() const {
    TrainerSummary out;

    // Names
    out.trainerName = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen);
    out.rivalName   = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::RivalNameOff,   Gen1Layout::RivalNameLen);

    // Trainer ID
    const u8 hi = buffer_.ReadU8(Gen1Layout::TrainerIdOff);
    const u8 lo = buffer_.ReadU8(Gen1Layout::TrainerIdOff + 1);
    out.trainerId = static_cast<u16>((hi << 8) | lo);
    
    // Money / Coins
    out.money = BcdCodec::ReadBcd3(buffer_, Gen1Layout::MoneyOff);
    out.coins = BcdCodec::ReadBcd2(buffer_, Gen1Layout::CoinsOff);

    // Badges
    out.badges = buffer_.ReadU8(Gen1Layout::BadgesOff);

    // Location
    out.mapId = buffer_.ReadU8(Gen1Layout::MapIdOff);
    out.x     = buffer_.ReadU8(Gen1Layout::XCoordOff);
    out.y     = buffer_.ReadU8(Gen1Layout::YCoordOff);

    // Playtime
    out.playHours   = buffer_.ReadU8(Gen1Layout::PlayTimeHoursOff);
    out.playMinutes = buffer_.ReadU8(Gen1Layout::PlayTimeMinutesOff);
    out.playSeconds = buffer_.ReadU8(Gen1Layout::PlayTimeSecondsOff);

    return out;
}

// Gen I full box layout refresher:
// Each box block is 0x462 bytes.
// - Count: 1 byte
// - Species list: 20 bytes (+ 0xFF terminator in practice)
// - Padding: 1 byte
// - Box Pokémon data: 20 entries * 0x21 bytes
// Level is stored inside the 0x21-byte "box Pokémon" struct.
// For MVP stats, we only compute count and average of the level byte.
BoxStats ReadOnlyData::GetBoxStats(int boxIndex1to12) const {
    if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
        throw std::out_of_range("GetBoxStats: box index must be 1..12");
    }

    BoxStats stats;
    stats.boxIndex = boxIndex1to12;

    const std::size_t base = Gen1Layout::BoxBaseOffsetByIndex1to12(boxIndex1to12);

    // Byte 0: count
    const int count = static_cast<int>(buffer_.ReadU8(base));
    stats.pokemonCount = std::clamp(count, 0, 20);

    if (stats.pokemonCount == 0) {
        stats.averageLevel = 0.0;
        return stats;
    }

    // Levels inside box Pokémon structs:
    // The 20 * 0x21 structs start after:
    // 1 (count) + 20 (species list) + 1 (padding) = 22 bytes = 0x16
    const std::size_t structsBase = base + 0x16;

    // We will assume the level is at offset 0x03 within each 0x21 struct.
    // NOTE: If later research shows a different offset, update this constant.
    constexpr std::size_t kBoxMonStructSize = 0x21;
    constexpr std::size_t kLevelOffsetInStruct = 0x03;

    int levelSum = 0;
    int levelCount = 0;

    for (int i = 0; i < stats.pokemonCount; ++i) {
        const std::size_t monBase = structsBase + static_cast<std::size_t>(i) * kBoxMonStructSize;
        const int level = static_cast<int>(buffer_.ReadU8(monBase + kLevelOffsetInStruct));
        // Sanity: level should be 1..100 typically
        if (level >= 1 && level <= 100) {
            levelSum += level;
            levelCount++;
        }
    }

    stats.averageLevel = (levelCount > 0) ? (static_cast<double>(levelSum) / static_cast<double>(levelCount)) : 0.0;
    return stats;
}

// Event flag summary:
// Bulbapedia lists a large completed-game-events bitfield (0x29F3, length 0x140).
// For MVP, we count set bits and list indices.
FlagSummary ReadOnlyData::GetEventFlagSummary() const {
    FlagSummary out;

    constexpr std::size_t kEventFlagsOff = 0x29F3;
    constexpr std::size_t kEventFlagsLen = 0x140;

    buffer_.RequireRange(kEventFlagsOff, kEventFlagsLen);

    int totalChecked = static_cast<int>(kEventFlagsLen) * 8;
    int totalSet = 0;

    for (std::size_t i = 0; i < kEventFlagsLen; ++i) {
        const u8 b = buffer_.ReadU8(kEventFlagsOff + i);
        totalSet += CountBits(b);

        if (b != 0) {
            for (int bit = 0; bit < 8; ++bit) {
                if (b & static_cast<u8>(1u << bit)) {
                    out.setFlagIndices.push_back(static_cast<int>(i * 8 + bit));
                }
            }
        }
    }

    out.totalFlagsChecked = totalChecked;
    out.totalFlagsSet = totalSet;

    return out;
}

std::string ReadOnlyData::DumpFullSummary() const {
    std::ostringstream oss;

    oss << "=== Save Genie Summary ===\n\n";

    const TrainerSummary t = GetTrainerSummary();
    oss << t.ToString() << "\n";

    // Checksums
    oss << "Main Checksum: " << (Gen1Checksum::ValidateMain(buffer_) ? "VALID" : "INVALID") << "\n";
    oss << "Bank2 All Checksum: " << (Gen1Checksum::ValidateBankAll(buffer_, 2) ? "VALID" : "INVALID") << "\n";
    oss << "Bank3 All Checksum: " << (Gen1Checksum::ValidateBankAll(buffer_, 3) ? "VALID" : "INVALID") << "\n\n";

    // Boxes (quick stats)
    oss << "--- PC Boxes (Stats) ---\n";
    for (int box = 1; box <= 12; ++box) {
        const BoxStats bs = GetBoxStats(box);
        oss << bs.ToString() << "\n";
    }
    oss << "\n";

    // Event flags
    oss << "--- Event Flags (Summary) ---\n";
    const FlagSummary fs = GetEventFlagSummary();
    oss << fs.ToString() << "\n";

    return oss.str();
}

int ReadOnlyData::CountBits(u8 byte) const {
    // Simple popcount (portable)
    int c = 0;
    for (int i = 0; i < 8; ++i) {
        if (byte & static_cast<u8>(1u << i)) c++;
    }
    return c;
}

} // namespace savegenie
