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
        oss << "Set Flag Indices (first 10): "; // 10 because its very long
        const std::size_t limit = std::min<std::size_t>(10, setFlagIndices.size());
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
// PokedexSummary
// =========================================================

std::string PokedexSummary::ToString() const {
    std::ostringstream oss;
    oss << "Owned: " << ownedCount << " / 151\n";
    oss << "Seen:  " << seenCount << " / 151\n";
    oss << "======================================\n";
    if (!ownedNames.empty()) {
        oss << "Owned List: ";
        const std::size_t limit = std::min<std::size_t>(152, ownedNames.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << ownedNames[i];
            if (i + 1 < limit) oss << ", ";
        }
        if (ownedNames.size() > limit) oss << " ...";
        oss << "\n";
    }
    
    oss << "======================================\n";
    if (!seenNames.empty()) {
        oss << "Seen List:  ";
        const std::size_t limit = std::min<std::size_t>(151, seenNames.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << seenNames[i];
            if (i + 1 < limit) oss << ", ";
        }
        if (seenNames.size() > limit) oss << " ...";
        oss << "\n";
    }
    
    oss << "======================================\n";


    return oss.str();
}

// =========================================================
// HallOfFamePokemon / HallOfFameEntry
// =========================================================

static bool IsLikelyValidGen1SpeciesId(savegenie::u8 speciesId) {
    // Strict-ish: real species are 1..151 in Gen I.
    // We keep this strict to avoid parsing junk as Hall of Fame.
    return speciesId >= 1 && speciesId <= 151;
}

static bool NameLooksReasonable(const std::string& s) {
    // Minimal heuristic: not empty, not mostly '?'
    if (s.empty()) return false;
    int q = 0;
    int nonSpace = 0;
    for (char c : s) {
        if (c == '?') q++;
        if (c != ' ') nonSpace++;
    }
    if (nonSpace == 0) return false;
    // If more than half are '?', it's probably not a real decoded name.
    return q * 2 < static_cast<int>(s.size());
}

std::string HallOfFamePokemon::ToString() const {
    std::ostringstream oss;
    oss << "Species ID=" << static_cast<int>(speciesId)
        << " Species Name: " << speciesName
        << " Lv " << static_cast<int>(level);

    if (!name.empty()) {
        oss << " \"" << name << "\"";
    }
    return oss.str();
}

std::string HallOfFameEntry::ToString() const {
    std::ostringstream oss;
    oss << "Entry #" << entryIndex << ":\n";
    for (std::size_t i = 0; i < team.size(); ++i) {
        oss << "  " << (i + 1) << ") " << team[i].ToString() << "\n";
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

PokedexSummary ReadOnlyData::GetPokedexSummary(bool includeNames) const {
    PokedexSummary out;

    // Read bitsets
    buffer_.RequireRange(Gen1Layout::PokedexOwnedOff, Gen1Layout::PokedexBitsLen);
    buffer_.RequireRange(Gen1Layout::PokedexSeenOff,  Gen1Layout::PokedexBitsLen);

    const auto ownedBytes = buffer_.Slice(Gen1Layout::PokedexOwnedOff, Gen1Layout::PokedexBitsLen);
    const auto seenBytes  = buffer_.Slice(Gen1Layout::PokedexSeenOff,  Gen1Layout::PokedexBitsLen);

    // 0x13 bytes = 152 bits; we use Dex #1..151.
    for (int dexNo = 1; dexNo <= 151; ++dexNo) {
        const int bitIndex = dexNo - 1;
        const int byteIndex = bitIndex / 8;
        const int bitInByte = bitIndex % 8;

        const u8 ownedB = ownedBytes[static_cast<std::size_t>(byteIndex)];
        const u8 seenB  = seenBytes[static_cast<std::size_t>(byteIndex)];

        const bool owned = (ownedB & static_cast<u8>(1u << bitInByte)) != 0;
        const bool seen  = (seenB  & static_cast<u8>(1u << bitInByte)) != 0;

        if (owned) {
            out.ownedCount++;
            out.ownedDexNos.push_back(dexNo);
        }
        if (seen) {
            out.seenCount++;
            out.seenDexNos.push_back(dexNo);
        }

        if (includeNames) {
            // DexNo -> internal SpeciesID -> name
            const int speciesId = Gen1SpeciesLookup::PokeDex[dexNo];
            const std::string name = (speciesId >= 0) ? Gen1SpeciesLookup::NameFromId(static_cast<u8>(speciesId)) : "INVALID";

            if (owned) out.ownedNames.push_back(name);
            if (seen)  out.seenNames.push_back(name);
        }
    }

    return out;
}

std::vector<HallOfFameEntry> ReadOnlyData::GetHallOfFame() const {
    // Hint count lives in Bank 1.
    const int rawCountHint = static_cast<int>(buffer_.ReadU8(Gen1Layout::HallOfFameRecordCountOff));
    const int countHint = std::clamp(rawCountHint, 0, Gen1Layout::HallOfFameMaxRecords);

    // Ensure the HoF block exists.
    buffer_.RequireRange(Gen1Layout::HallOfFameOff, Gen1Layout::HallOfFameLen);

    std::vector<HallOfFameEntry> valid;
    valid.reserve(Gen1Layout::HallOfFameMaxRecords);

    // Bank 0 is not checksum-protected; scan all 50 records and validate.
    for (int i = 0; i < Gen1Layout::HallOfFameMaxRecords; ++i) {
        const std::size_t recordOff = Gen1Layout::HallOfFameOff
            + static_cast<std::size_t>(i) * Gen1Layout::HallOfFameRecordSize;

        // Defensive range check.
        buffer_.RequireRange(recordOff, Gen1Layout::HallOfFameRecordSize);

        HallOfFameEntry entry;
        entry.entryIndex = i + 1;

        for (int j = 0; j < Gen1Layout::HallOfFameMonsPerRecord; ++j) {
            const std::size_t monOff = recordOff + static_cast<std::size_t>(j) * Gen1Layout::HallOfFameMonEntrySize;
            buffer_.RequireRange(monOff, Gen1Layout::HallOfFameMonEntrySize);

            const u8 species = buffer_.ReadU8(monOff + 0x00);
            const u8 level   = buffer_.ReadU8(monOff + 0x01);

            // Empty slot heuristics
            if (species == 0x00 || species == 0xFF) {
                break;
            }

            // Validate: species + level
            if (!IsLikelyValidGen1SpeciesId(species)) {
                // If the first slot is invalid, this is almost certainly junk.
                // Stop reading further slots in this record.
                if (j == 0) {
                    entry.team.clear();
                    break;
                }
                continue;
            }

            if (level < 1 || level > 100) {
                if (j == 0) {
                    entry.team.clear();
                    break;
                }
                continue;
            }

            HallOfFamePokemon mon;
            mon.speciesId = species;
            mon.speciesName = Gen1SpeciesLookup::NameFromId(static_cast<int>(species));
            mon.level = level;
            mon.name = Gen1TextCodec::DecodeName(buffer_, monOff + 0x02, 0x0B);

            // Optional name sanity (helps reject junk)
            if (!NameLooksReasonable(mon.name)) {
                if (j == 0) {
                    entry.team.clear();
                    break;
                }
                continue;
            }

            entry.team.push_back(std::move(mon));
        }

        if (!entry.team.empty()) {
            valid.push_back(std::move(entry));
        }
    }

    // If the game says 0, show nothing (your requirement).
    if (countHint == 0) {
        return {};
    }

    // Prefer showing the newest `countHint` valid entries.
    if (static_cast<int>(valid.size()) <= countHint) {
        // Renumber entries for display (1..N)
        for (std::size_t i = 0; i < valid.size(); ++i) {
            valid[i].entryIndex = static_cast<int>(i + 1);
        }
        return valid;
    }

    std::vector<HallOfFameEntry> out;
    out.reserve(static_cast<std::size_t>(countHint));

    const std::size_t start = valid.size() - static_cast<std::size_t>(countHint);
    for (std::size_t i = start; i < valid.size(); ++i) {
        valid[i].entryIndex = static_cast<int>(out.size() + 1);
        out.push_back(std::move(valid[i]));
    }

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
    oss << "Bank3 All Checksum: " << (Gen1Checksum::ValidateBankAll(buffer_, 3) ? "VALID" : "INVALID") << "\n";

    // Pokédex
    oss << "--- Pokédex ---\n";
    const PokedexSummary pdx = GetPokedexSummary(true);
    oss << pdx.ToString() << "\n";

    // Hall of Fame (only if present and record-count hint > 0)
    const auto hof = GetHallOfFame();
    if (!hof.empty()) {
        oss << "--- Hall of Fame ---\n";
        for (const auto& entry : hof) {
            oss << entry.ToString();
        }
        oss << "\n";
    }

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

