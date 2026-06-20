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
#include <cctype>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <fstream>

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
    oss << "Named Flags Known: " << namedFlagsKnown << "\n";
    oss << "Named Flags Set:   " << namedFlagsSet << "\n";
    oss << "Beat/Trainer Flags Set: " << beatFlagsSet << "\n";

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

    if (!namedSetFlags.empty()) {
        oss << "Named Set Flags (first 30):\n";
        const std::size_t limit = std::min<std::size_t>(30, namedSetFlags.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << "  " << namedSetFlags[i].name
                << " = true"
                << " (Flag " << namedSetFlags[i].index << ")\n";
        }
        if (namedSetFlags.size() > limit) {
            oss << "  ... " << (namedSetFlags.size() - limit)
                << " more named set flags\n";
        }
    }

    if (!namedFlags.empty()) {
        oss << "All Named Event Flags (pret/pokered true/false list):\n";
        for (const auto& flag : namedFlags) {
            oss << "  " << flag.name
                << " = " << (flag.isSet ? "true" : "false")
                << " (Flag " << flag.index << ")\n";
        }
    }

    return oss.str();
}

// =========================================================
// EventCategorySummary
// =========================================================

std::string EventCategorySummary::ToString() const {
    std::ostringstream oss;
    oss << "Defeated Trainer Flags Set: " << defeatedTrainerFlagsSet << "\n";
    oss << "Trainer Flags Known:        " << trainerFlagsKnown << "\n";
    oss << "Trainer Flags Complete:     " << trainerFlagsComplete << "\n";
    oss << "Story Flags Known:          " << storyFlagsKnown << "\n";
    oss << "Story Flags Complete:       " << storyFlagsComplete << "\n";
    oss << "Static Battle Flags Known:  " << staticEncounterFlagsKnown << "\n";
    oss << "Static Battle Complete:     " << staticEncounterFlagsComplete << "\n";
    oss << "Gym Leader Flags Set:       " << gymLeaderFlagsSet << "\n";
    oss << "Major Story Milestones Set: " << majorStoryMilestonesSet << "\n";
    oss << "Legendary Flags Set:        " << legendaryFlagsSet << "\n";
    oss << "Gym/Badge Mismatches:       " << gymStoryMismatchCount << "\n";
    oss << "Completion meaning: true = finished/complete, false = incomplete/not set\n";

    if (!gymConsistency.empty()) {
        oss << "Gym / Badge Consistency:\n";
        for (const auto& gym : gymConsistency) {
            oss << "  " << gym.badgeName
                << ": badge=" << (gym.badgeOwned ? "yes" : "no")
                << ", leaderEvent=" << (gym.leaderEventSet ? "yes" : "no")
                << ", " << (gym.consistent ? "consistent" : "MISMATCH")
                << " (" << gym.eventName << ")\n";
        }
    }

    if (!trainerFlags.empty()) {
        oss << "Trainer Flags (pret/pokered true/false list):\n";
        for (const auto& flag : trainerFlags) {
            oss << "  " << flag.label
                << ": " << (flag.isComplete ? "true" : "false")
                << " (" << flag.eventName << " / Flag " << flag.index << ")\n";
        }
    }

    if (!staticEncounterFlags.empty()) {
        oss << "Static / One-off Battle Flags (pret/pokered true/false list):\n";
        for (const auto& flag : staticEncounterFlags) {
            oss << "  " << flag.label
                << ": " << (flag.isComplete ? "true" : "false")
                << " (" << flag.eventName << " / Flag " << flag.index << ")\n";
        }
    }

    if (!storyFlags.empty()) {
        oss << "Story / World Event Flags (pret/pokered true/false list):\n";
        for (const auto& flag : storyFlags) {
            oss << "  [" << flag.category << "] " << flag.label
                << ": " << (flag.isComplete ? "true" : "false")
                << " (" << flag.eventName << " / Flag " << flag.index << ")\n";
        }
    }

    if (!majorStoryMilestones.empty()) {
        oss << "Major Story Milestones (first 20):\n";
        const std::size_t limit = std::min<std::size_t>(20, majorStoryMilestones.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << "  " << majorStoryMilestones[i] << "\n";
        }
        if (majorStoryMilestones.size() > limit) {
            oss << "  ... " << (majorStoryMilestones.size() - limit)
                << " more major milestones\n";
        }
    }

    if (!legendaryFlags.empty()) {
        oss << "Legendary / One-off Battle Flags:\n";
        for (const auto& name : legendaryFlags) {
            oss << "  " << name << "\n";
        }
    }

    if (!defeatedTrainerFlags.empty()) {
        oss << "Defeated Trainer Flags (first 20):\n";
        const std::size_t limit = std::min<std::size_t>(20, defeatedTrainerFlags.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << "  " << defeatedTrainerFlags[i] << "\n";
        }
        if (defeatedTrainerFlags.size() > limit) {
            oss << "  ... " << (defeatedTrainerFlags.size() - limit)
                << " more defeated-trainer flags\n";
        }
    }

    return oss.str();
}

// =========================================================
// PlayerStateSummary
// =========================================================

std::string PlayerStateSummary::MovementModeName() const {
    switch (walkBikeSurf) {
        case 0x00: return "Walking";
        case 0x01: return "Biking";
        case 0x02: return "Surfing";
        default: return "Unknown";
    }
}

std::string PlayerStateSummary::DirectionName(u8 dir) const {
    switch (dir) {
        case 0x00: return "None";
        case 0x01: return "Right";
        case 0x02: return "Left";
        case 0x04: return "Down";
        case 0x08: return "Up";
        default: return "Unknown";
    }
}

std::string PlayerStateSummary::ToString() const {
    std::ostringstream oss;
    oss << "Options Byte:        0x" << std::uppercase << std::hex
        << std::setw(2) << std::setfill('0') << static_cast<int>(optionsByte)
        << std::dec << std::setfill(' ') << "\n";
    oss << "Letter Delay Byte:   0x" << std::uppercase << std::hex
        << std::setw(2) << std::setfill('0') << static_cast<int>(letterDelayByte)
        << std::dec << std::setfill(' ') << "\n";
    oss << "Contrast:            " << static_cast<int>(contrast) << "\n";
    oss << "Block Coordinates:   X=" << static_cast<int>(xBlockCoord)
        << " Y=" << static_cast<int>(yBlockCoord) << "\n";
    oss << "Move Direction:      " << DirectionName(playerMoveDir)
        << " (" << static_cast<int>(playerMoveDir) << ")\n";
    oss << "Last Stop Direction: " << DirectionName(playerLastStopDir)
        << " (" << static_cast<int>(playerLastStopDir) << ")\n";
    oss << "Current Direction:   " << DirectionName(playerCurDir)
        << " (" << static_cast<int>(playerCurDir) << ")\n";
    oss << "Movement Mode:       " << MovementModeName()
        << " (" << static_cast<int>(walkBikeSurf) << ")\n";
    oss << "Special Warp Offset: X=" << static_cast<int>(specialWarpX)
        << " Y=" << static_cast<int>(specialWarpY) << "\n";
    oss << "Safari:              gameOver=" << (safariGameOver ? "yes" : "no")
        << ", balls=" << static_cast<int>(safariBallCount)
        << ", steps=" << safariSteps << "\n";
    oss << "Player Flags:        strength=" << (strengthOutsideBattle ? "yes" : "no")
        << ", surf=" << (surfingAllowed ? "yes" : "no")
        << ", fly=" << (flyOutOfBattle ? "yes" : "no")
        << ", cardKey=" << (usedCardKey ? "yes" : "no") << "\n";
    oss << "Runtime Flags:       battle=" << (isBattle ? "yes" : "no")
        << ", trainerBattle=" << (isTrainerBattle ? "yes" : "no")
        << ", noBattles=" << (noBattles ? "yes" : "no")
        << ", linkCable=" << (usingLinkCable ? "yes" : "no") << "\n";
    oss << "Door/Warp Flags:     standingDoor=" << (standingOnDoor ? "yes" : "no")
        << ", movingDoor=" << (movingThroughDoor ? "yes" : "no")
        << ", standingWarp=" << (standingOnWarp ? "yes" : "no") << "\n";
    oss << "Text/Playtime Flags: noLetterDelay=" << (noLetterDelay ? "yes" : "no")
        << ", countPlaytime=" << (countPlaytime ? "yes" : "no") << "\n";

    return oss.str();
}

// =========================================================
// WorldStateSummary
// =========================================================

std::string WorldStateSummary::ToString() const {
    std::ostringstream oss;
    oss << "Missable Object Flags Set: " << missableObjectsSet
        << " / " << missableObjectsChecked << "\n";
    oss << "Hidden Items Collected:    " << hiddenItemsCollected
        << " / " << hiddenItemsChecked << "\n";
    oss << "Hidden Coins Collected:    " << hiddenCoinsCollected
        << " / " << hiddenCoinsChecked << "\n";
    oss << "Visited Towns:             " << visitedTownsSet
        << " / " << visitedTownsChecked << "\n";
    oss << "Current Script Bytes Nonzero: " << currentScriptsNonZero
        << " / " << currentScriptsChecked << "\n";

    const auto printIndices = [&oss](const char* label, const std::vector<int>& values) {
        if (values.empty()) return;
        oss << label << ": ";
        for (std::size_t i = 0; i < values.size(); ++i) {
            oss << values[i];
            if (i + 1 < values.size()) oss << ", ";
        }
        oss << "\n";
    };

    printIndices("First Set Missable Indices", firstSetMissableObjects);
    printIndices("First Hidden Item Indices", firstCollectedHiddenItems);
    printIndices("First Hidden Coin Indices", firstCollectedHiddenCoins);
    printIndices("Visited Town Indices", visitedTownIndices);
    printIndices("First Nonzero Script Byte Indices", nonZeroCurrentScriptIndices);

    oss << "Story/Misc Bits:\n";
    oss << "  Got Old Rod:              " << (gotOldRod ? "yes" : "no") << "\n";
    oss << "  Got Good Rod:             " << (gotGoodRod ? "yes" : "no") << "\n";
    oss << "  Got Super Rod:            " << (gotSuperRod ? "yes" : "no") << "\n";
    oss << "  Satisfied Saffron Guards: " << (satisfiedSaffronGuards ? "yes" : "no") << "\n";
    oss << "  Got Lapras:               " << (gotLapras ? "yes" : "no") << "\n";
    oss << "  Ever Healed Pokemon:      " << (everHealedPokemon ? "yes" : "no") << "\n";
    oss << "  Got Starter:              " << (gotStarter ? "yes" : "no") << "\n";
    oss << "  Defeated Lorelei:         " << (defeatedLorelei ? "yes" : "no") << "\n";

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
        << " Pokedex ID= " << Gen1SpeciesLookup::DexfromId(speciesId)
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
// BagItem / BagSummary
// =========================================================

std::string BagItem::ToString() const {
    std::ostringstream oss;

    // Example output:
    //  - POTION (0x14) x12
    //  - INVALID (0x7A) x3
    if (!itemName.empty()) oss << itemName;
    else oss << "INVALID";

    if (!itemHex.empty()) oss << " (" << itemHex << ")";
    else {
        // Fallback hex if not provided
        static const char* kHex = "0123456789ABCDEF";
        oss << " (0x";
        oss << kHex[(itemId >> 4) & 0xF] << kHex[itemId & 0xF];
        oss << ")";
    }

    oss << " x" << static_cast<int>(quantity);
    return oss.str();
}

std::string BagSummary::ToString() const {
    std::ostringstream oss;

    oss << "Item Count: " << itemCount << "\n"; // Types of items
    for (std::size_t i = 0; i < items.size(); ++i) {
        oss << "  " << (i + 1) << ") " << items[i].ToString() << "\n";
    }
    return oss.str();
}

// =========================================================
// Pokémon Models (Party + PC Boxes)
// =========================================================

std::string PokemonMove::ToString() const {
    std::ostringstream oss;
    if (!moveName.empty()) {
        oss << moveName;
    } else {
        oss << "MoveID=" << static_cast<int>(moveId);
    }
    oss << " (PP " << static_cast<int>(ppCurrent) << "/" << static_cast<int>(ppMax) << ")";
    return oss.str();
}

std::string PokemonStats::ToString() const {
    std::ostringstream oss;
    // Keep this compact for terminal output.
    oss << "HP " << hpCurrent << "/" << hpMax
        << " | ATK " << attack
        << " | DEF " << defense
        << " | SPD " << speed
        << " | SPC " << special
        << " | Status 0x" << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(status) << std::dec;
    return oss.str();
}

std::string PokemonMon::ToString() const {
    std::ostringstream oss;

    oss << "[" << position << "] "
        << (speciesName.empty() ? "UNKNOWN" : speciesName)
        << " (InternalID=" << static_cast<int>(speciesId);

    if (!dexNo.empty() && dexNo != "INVALID") {
        oss << " | Dex#" << dexNo;
    }
    oss << ")\n";

    oss << "    Nickname: \"" << nickname << "\"\n";
    oss << "    OT:       \"" << otName << "\" (IDNo: " << otIdNo << ")\n";
    oss << "    Level:    " << static_cast<int>(level) << "\n";
    oss << "    EXP:      " << expPoints << "\n";

    // Hidden Gen I values
    oss << "    DVs:      HP " << static_cast<int>(dvHP)
        << " | ATK " << static_cast<int>(dvAtk)
        << " | DEF " << static_cast<int>(dvDef)
        << " | SPD " << static_cast<int>(dvSpd)
        << " | SPC " << static_cast<int>(dvSpc) << "\n";

    oss << "    StatExp:  HP " << statExpHP
        << " | ATK " << statExpAtk
        << " | DEF " << statExpDef
        << " | SPD " << statExpSpd
        << " | SPC " << statExpSpc << "\n";

    // Stats line (HP/stats/status). For box mons, hpCurrent/hpMax/status may be 0.
    oss << "    Stats:    " << stats.ToString() << "\n";

    if (!moves.empty()) {
        oss << "    Moves:\n";
        const std::size_t limit = std::min<std::size_t>(4, moves.size());
        for (std::size_t i = 0; i < limit; ++i) {
            oss << "      " << (i + 1) << ") " << moves[i].ToString() << "\n";
        }
    }

    return oss.str();
}

std::string PokemonBox::ToString() const {
    std::ostringstream oss;
    oss << "Box " << boxNumber;
    if (!label.empty()) oss << " (" << label << ")";
    oss << " - Count: " << pokemonCount << "\n";

    for (const auto& mon : pokemon) {
        oss << mon.ToString();
    }

    return oss.str();
}

std::string PokemonBoxesExport::ToString() const {
    std::ostringstream oss;
    oss << "PokemonBoxesExport: " << boxes.size() << " boxes\n";
    for (const auto& b : boxes) {
        oss << b.ToString() << "\n";
    }
    if (hasCurrentBoxCache) {
        oss << currentBoxCache.ToString() << "\n";
    }
    return oss.str();
}

std::string DaycareSummary::ToString() const {
    std::ostringstream oss;
    oss << "Daycare In Use: " << (inUse ? "yes" : "no") << "\n";

    if (inUse) {
        oss << "Pokemon:\n";
        oss << pokemon.ToString();
    }

    return oss.str();
}

// =========================================================
// ReadOnlyData::GetBagSummary
// =========================================================

BagSummary ReadOnlyData::GetBagSummary(bool includeNamesAndHex) const {
    BagSummary out;

    // Bag list lives in Bank 1 main data
    buffer_.RequireRange(Gen1Layout::BagItemsOff, Gen1Layout::BagItemsLen);

    const u8 rawCount = buffer_.ReadU8(Gen1Layout::BagItemsCountOff);
    // Defensive clamp: Gen I bag typically supports up to 20 items
    out.itemCount = std::clamp<int>(static_cast<int>(rawCount), 0, Gen1Layout::BagItemsMaxPairs);

    std::size_t off = Gen1Layout::BagItemsPairsOff;

    for (int i = 0; i < out.itemCount; ++i) {
        // Each entry is (itemId, qty)
        buffer_.RequireRange(off, 2);

        const u8 itemId = buffer_.ReadU8(off + 0);
        const u8 qty    = buffer_.ReadU8(off + 1);

        // Defensive stop: many Gen I lists terminate with 0xFF
        if (itemId == 0xFF) break;

        BagItem bi;
        bi.itemId = itemId;
        bi.quantity = qty;

        if (includeNamesAndHex) {
            bi.itemName = Gen1ItemLookup::NameFromId(itemId);
            bi.itemHex  = Gen1ItemLookup::ItemHex[static_cast<std::size_t>(itemId)];
        }

        out.items.push_back(std::move(bi));
        off += 2;
    }

    return out;
}


// =========================================================
// PC Item Box Summary
// =========================================================
BagSummary ReadOnlyData::GetPCItemBoxSummary(bool includeNamesAndHex) const {
    BagSummary out;

    buffer_.RequireRange(Gen1Layout::PCItemBoxOff, Gen1Layout::PCItemBoxLen);

    const u8 rawCount = buffer_.ReadU8(Gen1Layout::PCItemBoxCountOff);
    out.itemCount = std::clamp<int>(static_cast<int>(rawCount), 0, Gen1Layout::PCItemBoxMaxPairs);

    std::size_t off = Gen1Layout::PCItemBoxPairsOff;

    for (int i = 0; i < out.itemCount; ++i) {
        buffer_.RequireRange(off, 2);

        const u8 itemId = buffer_.ReadU8(off + 0);
        const u8 qty    = buffer_.ReadU8(off + 1);

        // Defensive stop: list terminator
        if (itemId == 0xFF) break;

        BagItem bi;
        bi.itemId = itemId;
        bi.quantity = qty;

        if (includeNamesAndHex) {
            bi.itemName = Gen1ItemLookup::NameFromId(itemId);
            bi.itemHex  = Gen1ItemLookup::ItemHex[static_cast<std::size_t>(itemId)];
        }

        out.items.push_back(std::move(bi));
        off += 2;
    }

    return out;
}

// =========================================================
// Pokémon decode helpers
// =========================================================

static u16 ReadU16BE_At(const SaveBuffer& b, std::size_t off) {
    const u8 hi = b.ReadU8(off);
    const u8 lo = b.ReadU8(off + 1);
    return static_cast<u16>((hi << 8) | lo);
}

static u32 ReadU24BE_At(const SaveBuffer& b, std::size_t off) {
    const u8 hi = b.ReadU8(off);
    const u8 mid = b.ReadU8(off + 1);
    const u8 lo = b.ReadU8(off + 2);
    return static_cast<u32>((hi << 16) | (mid << 8) | lo);
}

// Decode a Gen I party Pokémon struct (size 0x2C) into a PokemonMon.
// Layout used here matches the commonly documented Gen I "party mon" struct:
//  0x00 species
//  0x01-0x02 current HP (BE)
//  0x03 level
//  0x04 status
//  0x08-0x0B moves (4)
//  0x1D-0x20 PP (4)
//  0x22-0x23 max HP (BE)
//  0x24-0x25 attack
//  0x26-0x27 defense
//  0x28-0x29 speed
//  0x2A-0x2B special
//  0x0E-0x10 EXP (BE, 3 bytes)
static void DecodePartyMonStruct(const SaveBuffer& buf, std::size_t monOff, PokemonMon& out) {
    buf.RequireRange(monOff, Gen1Layout::PartyStructSize);

    out.speciesId = buf.ReadU8(monOff + 0x00);

    out.stats.hpCurrent = ReadU16BE_At(buf, monOff + 0x01);
    out.level = buf.ReadU8(monOff + 0x21);
    
    out.stats.status = buf.ReadU8(monOff + 0x04);

    out.otIdNo = ReadU16BE_At(buf, monOff + 0x0C);
    out.expPoints = ReadU24BE_At(buf, monOff + 0x0E);

    // Stat Exp (Gen I EV-like values): 5 stats, 2 bytes each.
    out.statExpHP  = ReadU16BE_At(buf, monOff + 0x11);
    out.statExpAtk = ReadU16BE_At(buf, monOff + 0x13);
    out.statExpDef = ReadU16BE_At(buf, monOff + 0x15);
    out.statExpSpd = ReadU16BE_At(buf, monOff + 0x17);
    out.statExpSpc = ReadU16BE_At(buf, monOff + 0x19);

    // DVs (Gen I IV-like values): packed at 0x1B..0x1C.
    const u8 dv1 = buf.ReadU8(monOff + 0x1B);
    const u8 dv2 = buf.ReadU8(monOff + 0x1C);

    out.dvAtk = static_cast<u8>((dv1 >> 4) & 0x0F);
    out.dvDef = static_cast<u8>((dv1 >> 0) & 0x0F);
    out.dvSpd = static_cast<u8>((dv2 >> 4) & 0x0F);
    out.dvSpc = static_cast<u8>((dv2 >> 0) & 0x0F);

    // HP DV is derived from the low bits of Atk/Def/Spd/Spc.
    out.dvHP = static_cast<u8>(((out.dvAtk & 1) << 3) | ((out.dvDef & 1) << 2) | ((out.dvSpd & 1) << 1) | ((out.dvSpc & 1) << 0));

    // Moves
    out.moves.clear();
    out.moves.reserve(4);
    for (int i = 0; i < 4; ++i) {
        PokemonMove mv;
        mv.moveId = buf.ReadU8(monOff + 0x08 + static_cast<std::size_t>(i));
        mv.moveName = Gen1MoveLookup::MoveFromId(mv.moveId);

        const u8 rawPP = buf.ReadU8(monOff + 0x1D + static_cast<std::size_t>(i));
        const u8 ppCur = static_cast<u8>(rawPP & 0x3F);  // lower 6 bits
        // upper 2 bits are PP Ups used (0..3): (rawPP >> 6)

        mv.ppCurrent = ppCur;
        mv.ppMax = ppCur; // placeholder until we add base PP table + PP Ups calculation

        out.moves.push_back(std::move(mv));
    }

    // Stats (max HP + atk/def/spd/spc)
    // These are stored at the end of the 0x2C party struct; use the +1-shifted offsets.
    out.stats.hpMax   = ReadU16BE_At(buf, monOff + 0x22);
    out.stats.attack  = ReadU16BE_At(buf, monOff + 0x24);
    out.stats.defense = ReadU16BE_At(buf, monOff + 0x26);
    out.stats.speed   = ReadU16BE_At(buf, monOff + 0x28);
    out.stats.special = ReadU16BE_At(buf, monOff + 0x2A);
}

// Decode a Gen I boxed Pokémon struct (size 0x21) into a PokemonMon.
// This is a truncated form of the party struct and typically does not contain live battle HP/status.
// We still extract level/exp/moves/PP when available.
static void DecodeBoxMonStruct(const SaveBuffer& buf, std::size_t monOff, PokemonMon& out) {
    buf.RequireRange(monOff, Gen1Layout::BoxStructSize);

    out.speciesId = buf.ReadU8(monOff + 0x00);

    // Many tools treat level at +0x03 for boxed mons.
    out.level = buf.ReadU8(monOff + 0x21);
    out.otIdNo = ReadU16BE_At(buf, monOff + 0x0C);
    out.expPoints = ReadU24BE_At(buf, monOff + 0x0E);

    // Stat Exp (Gen I EV-like values): 5 stats, 2 bytes each.
    out.statExpHP  = ReadU16BE_At(buf, monOff + 0x11);
    out.statExpAtk = ReadU16BE_At(buf, monOff + 0x13);
    out.statExpDef = ReadU16BE_At(buf, monOff + 0x15);
    out.statExpSpd = ReadU16BE_At(buf, monOff + 0x17);
    out.statExpSpc = ReadU16BE_At(buf, monOff + 0x19);

    // DVs (Gen I IV-like values): packed at 0x1B..0x1C.
    const u8 dv1 = buf.ReadU8(monOff + 0x1B);
    const u8 dv2 = buf.ReadU8(monOff + 0x1C);

    out.dvAtk = static_cast<u8>((dv1 >> 4) & 0x0F);
    out.dvDef = static_cast<u8>((dv1 >> 0) & 0x0F);
    out.dvSpd = static_cast<u8>((dv2 >> 4) & 0x0F);
    out.dvSpc = static_cast<u8>((dv2 >> 0) & 0x0F);

    // HP DV is derived from the low bits of Atk/Def/Spd/Spc.
    out.dvHP = static_cast<u8>(((out.dvAtk & 1) << 3) | ((out.dvDef & 1) << 2) | ((out.dvSpd & 1) << 1) | ((out.dvSpc & 1) << 0));

    // Moves + PP
    out.moves.clear();
    out.moves.reserve(4);
    for (int i = 0; i < 4; ++i) {
        PokemonMove mv;
        mv.moveId = buf.ReadU8(monOff + 0x08 + static_cast<std::size_t>(i));
        mv.moveName = Gen1MoveLookup::MoveFromId(mv.moveId);

        const u8 rawPP = buf.ReadU8(monOff + 0x1D + static_cast<std::size_t>(i));
        const u8 ppCur = static_cast<u8>(rawPP & 0x3F);  // lower 6 bits
        // upper 2 bits are PP Ups used (0..3): (rawPP >> 6)

        mv.ppCurrent = ppCur;
        mv.ppMax = ppCur; // placeholder until we add base PP table + PP Ups calculation

        out.moves.push_back(std::move(mv));
    }

    // Box structs do not store maxHP/atk/def/spd/spc in the same way; leave 0 for now.
    out.stats.hpCurrent = 0;
    out.stats.hpMax = 0;
    out.stats.status = 0;
    out.stats.attack = 0;
    out.stats.defense = 0;
    out.stats.speed = 0;
    out.stats.special = 0;
}

static PokemonBox DecodeBoxBlock(
    const SaveBuffer& buffer,
    std::size_t base,
    int boxNumber,
    const std::string& label
) {
    PokemonBox box;
    box.boxNumber = boxNumber;
    box.label = label;

    buffer.RequireRange(base, Gen1Layout::BoxBlockSize);

    const int rawCount = static_cast<int>(buffer.ReadU8(base + Gen1Layout::BoxCountRel));
    box.pokemonCount = std::clamp(rawCount, 0, Gen1Layout::BoxMaxMons);

    for (int i = 0; i < box.pokemonCount; ++i) {
        PokemonMon mon;
        mon.position = i + 1;

        const u8 speciesId = buffer.ReadU8(base + Gen1Layout::BoxSpeciesRel + static_cast<std::size_t>(i));
        mon.speciesId = speciesId;
        mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
        mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);

        mon.otName = Gen1TextCodec::DecodeName(buffer, base + Gen1Layout::BoxOTNamesRel + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);
        mon.nickname = Gen1TextCodec::DecodeName(buffer, base + Gen1Layout::BoxNicknamesRel + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);

        const std::size_t monStructOff = base + Gen1Layout::BoxStructsRel + static_cast<std::size_t>(i) * Gen1Layout::BoxStructSize;
        DecodeBoxMonStruct(buffer, monStructOff, mon);

        if (mon.speciesId == 0) {
            mon.speciesId = speciesId;
            mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
            mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);
        }

        box.pokemon.push_back(std::move(mon));
    }

    return box;
}

PokemonBox ReadOnlyData::GetPartyAsBox0() const {
    PokemonBox box;
    box.boxNumber = 0;
    box.label = "Party";

    buffer_.RequireRange(Gen1Layout::PartyBase, Gen1Layout::PartyBlockLen);

    const int rawCount = static_cast<int>(buffer_.ReadU8(Gen1Layout::PartyCountOff));
    box.pokemonCount = std::clamp(rawCount, 0, Gen1Layout::PartyMaxMons);

    for (int i = 0; i < box.pokemonCount; ++i) {
        PokemonMon mon;
        mon.position = i + 1;

        // Species list
        const u8 speciesId = buffer_.ReadU8(Gen1Layout::PartySpeciesOff + static_cast<std::size_t>(i));
        mon.speciesId = speciesId;
        mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
        mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);

        // Names
        mon.otName = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::PartyOTNamesOff + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);
        mon.nickname = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::PartyNicknamesOff + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);

        // Struct fields
        const std::size_t monStructOff = Gen1Layout::PartyStructsOff + static_cast<std::size_t>(i) * Gen1Layout::PartyStructSize;
        DecodePartyMonStruct(buffer_, monStructOff, mon);

        // Overwrite speciesId from species list (defensive) if struct contains 0.
        if (mon.speciesId == 0) {
            mon.speciesId = speciesId;
            mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
            mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);
        }

        box.pokemon.push_back(std::move(mon));
    }

    return box;
}

PokemonBox ReadOnlyData::GetPCBox(int boxIndex1to12) const {
    if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
        throw std::out_of_range("GetPCBox: box index must be 1..12");
    }

    const std::size_t base = Gen1Layout::BoxBaseOffsetByIndex1to12(boxIndex1to12);
    return DecodeBoxBlock(buffer_, base, boxIndex1to12, std::string("PC Box ") + std::to_string(boxIndex1to12));
}

PokemonBox ReadOnlyData::GetCurrentBoxCache() const {
    return DecodeBoxBlock(buffer_, Gen1Layout::CurrentBoxCacheOff, -1, "Current Box Cache");
}

PokemonBoxesExport ReadOnlyData::GetAllBoxesExport() const {
    PokemonBoxesExport out;
    out.boxes.reserve(13);

    out.boxes.push_back(GetPartyAsBox0());
    for (int i = 1; i <= 12; ++i) {
        out.boxes.push_back(GetPCBox(i));
    }
    out.currentBoxCache = GetCurrentBoxCache();
    out.hasCurrentBoxCache = true;

    return out;
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

namespace {

struct EventFlagLabel {
    int index;
    const char* name;
};

static bool IsBeatEventName(const char* name) {
    return std::string(name).rfind("EVENT_BEAT_", 0) == 0;
}

// Source: pret/pokered constants/event_constants.asm, parsed from const_def,
// const_next, const_skip, and const macros on 2026-06-20.
static constexpr EventFlagLabel kEventFlagLabels[] = {
    {0, "EVENT_FOLLOWED_OAK_INTO_LAB"},
    {3, "EVENT_HALL_OF_FAME_DEX_RATING"},
    {6, "EVENT_PALLET_AFTER_GETTING_POKEBALLS"},
    {24, "EVENT_GOT_TOWN_MAP"},
    {25, "EVENT_ENTERED_BLUES_HOUSE"},
    {26, "EVENT_DAISY_WALKING"},
    {32, "EVENT_FOLLOWED_OAK_INTO_LAB_2"},
    {33, "EVENT_OAK_ASKED_TO_CHOOSE_MON"},
    {34, "EVENT_GOT_STARTER"},
    {35, "EVENT_BATTLED_RIVAL_IN_OAKS_LAB"},
    {36, "EVENT_GOT_POKEBALLS_FROM_OAK"},
    {37, "EVENT_GOT_POKEDEX"},
    {38, "EVENT_PALLET_AFTER_GETTING_POKEBALLS_2"},
    {39, "EVENT_OAK_APPEARED_IN_PALLET"},
    {40, "EVENT_VIRIDIAN_GYM_OPEN"},
    {41, "EVENT_GOT_TM42"},
    {56, "EVENT_OAK_GOT_PARCEL"},
    {57, "EVENT_GOT_OAKS_PARCEL"},
    {80, "EVENT_GOT_TM27"},
    {81, "EVENT_BEAT_VIRIDIAN_GYM_GIOVANNI"},
    {82, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_0"},
    {83, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_1"},
    {84, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_2"},
    {85, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_3"},
    {86, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_4"},
    {87, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_5"},
    {88, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_6"},
    {89, "EVENT_BEAT_VIRIDIAN_GYM_TRAINER_7"},
    {104, "EVENT_BOUGHT_MUSEUM_TICKET"},
    {105, "EVENT_GOT_OLD_AMBER"},
    {114, "EVENT_BEAT_PEWTER_GYM_TRAINER_0"},
    {118, "EVENT_GOT_TM34"},
    {119, "EVENT_BEAT_BROCK"},
    {152, "EVENT_BEAT_CERULEAN_RIVAL"},
    {167, "EVENT_BEAT_CERULEAN_ROCKET_THIEF"},
    {186, "EVENT_BEAT_CERULEAN_GYM_TRAINER_0"},
    {187, "EVENT_BEAT_CERULEAN_GYM_TRAINER_1"},
    {190, "EVENT_GOT_TM11"},
    {191, "EVENT_BEAT_MISTY"},
    {192, "EVENT_GOT_BICYCLE"},
    {238, "EVENT_POKEMON_TOWER_RIVAL_ON_LEFT"},
    {239, "EVENT_BEAT_POKEMON_TOWER_RIVAL"},
    {241, "EVENT_BEAT_POKEMONTOWER_3_TRAINER_0"},
    {242, "EVENT_BEAT_POKEMONTOWER_3_TRAINER_1"},
    {243, "EVENT_BEAT_POKEMONTOWER_3_TRAINER_2"},
    {249, "EVENT_BEAT_POKEMONTOWER_4_TRAINER_0"},
    {250, "EVENT_BEAT_POKEMONTOWER_4_TRAINER_1"},
    {251, "EVENT_BEAT_POKEMONTOWER_4_TRAINER_2"},
    {258, "EVENT_BEAT_POKEMONTOWER_5_TRAINER_0"},
    {259, "EVENT_BEAT_POKEMONTOWER_5_TRAINER_1"},
    {260, "EVENT_BEAT_POKEMONTOWER_5_TRAINER_2"},
    {261, "EVENT_BEAT_POKEMONTOWER_5_TRAINER_3"},
    {263, "EVENT_IN_PURIFIED_ZONE"},
    {265, "EVENT_BEAT_POKEMONTOWER_6_TRAINER_0"},
    {266, "EVENT_BEAT_POKEMONTOWER_6_TRAINER_1"},
    {267, "EVENT_BEAT_POKEMONTOWER_6_TRAINER_2"},
    {271, "EVENT_BEAT_GHOST_MAROWAK"},
    {273, "EVENT_BEAT_POKEMONTOWER_7_TRAINER_0"},
    {274, "EVENT_BEAT_POKEMONTOWER_7_TRAINER_1"},
    {275, "EVENT_BEAT_POKEMONTOWER_7_TRAINER_2"},
    {279, "EVENT_RESCUED_MR_FUJI_2"},
    {296, "EVENT_GOT_POKE_FLUTE"},
    {337, "EVENT_GOT_BIKE_VOUCHER"},
    {342, "EVENT_SEEL_FAN_BOAST"},
    {343, "EVENT_PIKACHU_FAN_BOAST"},
    {352, "EVENT_2ND_LOCK_OPENED"},
    {353, "EVENT_1ST_LOCK_OPENED"},
    {354, "EVENT_BEAT_VERMILION_GYM_TRAINER_0"},
    {355, "EVENT_BEAT_VERMILION_GYM_TRAINER_1"},
    {356, "EVENT_BEAT_VERMILION_GYM_TRAINER_2"},
    {358, "EVENT_GOT_TM24"},
    {359, "EVENT_BEAT_LT_SURGE"},
    {384, "EVENT_GOT_TM41"},
    {396, "EVENT_GOT_TM13"},
    {397, "EVENT_GOT_TM48"},
    {398, "EVENT_GOT_TM49"},
    {399, "EVENT_GOT_TM18"},
    {424, "EVENT_GOT_TM21"},
    {425, "EVENT_BEAT_ERIKA"},
    {426, "EVENT_BEAT_CELADON_GYM_TRAINER_0"},
    {427, "EVENT_BEAT_CELADON_GYM_TRAINER_1"},
    {428, "EVENT_BEAT_CELADON_GYM_TRAINER_2"},
    {429, "EVENT_BEAT_CELADON_GYM_TRAINER_3"},
    {430, "EVENT_BEAT_CELADON_GYM_TRAINER_4"},
    {431, "EVENT_BEAT_CELADON_GYM_TRAINER_5"},
    {432, "EVENT_BEAT_CELADON_GYM_TRAINER_6"},
    {440, "EVENT_1B8"},
    {441, "EVENT_FOUND_ROCKET_HIDEOUT"},
    {442, "EVENT_GOT_10_COINS"},
    {443, "EVENT_GOT_20_COINS"},
    {444, "EVENT_GOT_20_COINS_2"},
    {447, "EVENT_1BF"},
    {480, "EVENT_GOT_COIN_CASE"},
    {568, "EVENT_GOT_HM04"},
    {569, "EVENT_GAVE_GOLD_TEETH"},
    {590, "EVENT_SAFARI_GAME_OVER"},
    {591, "EVENT_IN_SAFARI_ZONE"},
    {600, "EVENT_GOT_TM06"},
    {601, "EVENT_BEAT_KOGA"},
    {602, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_0"},
    {603, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_1"},
    {604, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_2"},
    {605, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_3"},
    {606, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_4"},
    {607, "EVENT_BEAT_FUCHSIA_GYM_TRAINER_5"},
    {632, "EVENT_MANSION_SWITCH_ON"},
    {649, "EVENT_BEAT_MANSION_1_TRAINER_0"},
    {664, "EVENT_GOT_TM38"},
    {665, "EVENT_BEAT_BLAINE"},
    {666, "EVENT_BEAT_CINNABAR_GYM_TRAINER_0"},
    {667, "EVENT_BEAT_CINNABAR_GYM_TRAINER_1"},
    {668, "EVENT_BEAT_CINNABAR_GYM_TRAINER_2"},
    {669, "EVENT_BEAT_CINNABAR_GYM_TRAINER_3"},
    {670, "EVENT_BEAT_CINNABAR_GYM_TRAINER_4"},
    {671, "EVENT_BEAT_CINNABAR_GYM_TRAINER_5"},
    {672, "EVENT_BEAT_CINNABAR_GYM_TRAINER_6"},
    {679, "EVENT_2A7"},
    {680, "EVENT_CINNABAR_GYM_GATE0_UNLOCKED"},
    {681, "EVENT_CINNABAR_GYM_GATE1_UNLOCKED"},
    {682, "EVENT_CINNABAR_GYM_GATE2_UNLOCKED"},
    {683, "EVENT_CINNABAR_GYM_GATE3_UNLOCKED"},
    {684, "EVENT_CINNABAR_GYM_GATE4_UNLOCKED"},
    {685, "EVENT_CINNABAR_GYM_GATE5_UNLOCKED"},
    {686, "EVENT_CINNABAR_GYM_GATE6_UNLOCKED"},
    {727, "EVENT_GOT_TM35"},
    {736, "EVENT_GAVE_FOSSIL_TO_LAB"},
    {737, "EVENT_LAB_STILL_REVIVING_FOSSIL"},
    {738, "EVENT_LAB_HANDING_OVER_FOSSIL_MON"},
    {832, "EVENT_GOT_TM31"},
    {848, "EVENT_DEFEATED_FIGHTING_DOJO"},
    {849, "EVENT_BEAT_KARATE_MASTER"},
    {850, "EVENT_BEAT_FIGHTING_DOJO_TRAINER_0"},
    {851, "EVENT_BEAT_FIGHTING_DOJO_TRAINER_1"},
    {852, "EVENT_BEAT_FIGHTING_DOJO_TRAINER_2"},
    {853, "EVENT_BEAT_FIGHTING_DOJO_TRAINER_3"},
    {854, "EVENT_GOT_HITMONLEE"},
    {855, "EVENT_GOT_HITMONCHAN"},
    {864, "EVENT_GOT_TM46"},
    {865, "EVENT_BEAT_SABRINA"},
    {866, "EVENT_BEAT_SAFFRON_GYM_TRAINER_0"},
    {867, "EVENT_BEAT_SAFFRON_GYM_TRAINER_1"},
    {868, "EVENT_BEAT_SAFFRON_GYM_TRAINER_2"},
    {869, "EVENT_BEAT_SAFFRON_GYM_TRAINER_3"},
    {870, "EVENT_BEAT_SAFFRON_GYM_TRAINER_4"},
    {871, "EVENT_BEAT_SAFFRON_GYM_TRAINER_5"},
    {872, "EVENT_BEAT_SAFFRON_GYM_TRAINER_6"},
    {919, "EVENT_SILPH_CO_RECEPTIONIST_AT_DESK"},
    {944, "EVENT_GOT_TM29"},
    {960, "EVENT_GOT_POTION_SAMPLE"},
    {984, "EVENT_GOT_HM05"},
    {994, "EVENT_BEAT_ROUTE_3_TRAINER_0"},
    {995, "EVENT_BEAT_ROUTE_3_TRAINER_1"},
    {996, "EVENT_BEAT_ROUTE_3_TRAINER_2"},
    {997, "EVENT_BEAT_ROUTE_3_TRAINER_3"},
    {998, "EVENT_BEAT_ROUTE_3_TRAINER_4"},
    {999, "EVENT_BEAT_ROUTE_3_TRAINER_5"},
    {1000, "EVENT_BEAT_ROUTE_3_TRAINER_6"},
    {1001, "EVENT_BEAT_ROUTE_3_TRAINER_7"},
    {1010, "EVENT_BEAT_ROUTE_4_TRAINER_0"},
    {1023, "EVENT_BOUGHT_MAGIKARP"},
    {1041, "EVENT_BEAT_ROUTE_6_TRAINER_0"},
    {1042, "EVENT_BEAT_ROUTE_6_TRAINER_1"},
    {1043, "EVENT_BEAT_ROUTE_6_TRAINER_2"},
    {1044, "EVENT_BEAT_ROUTE_6_TRAINER_3"},
    {1045, "EVENT_BEAT_ROUTE_6_TRAINER_4"},
    {1046, "EVENT_BEAT_ROUTE_6_TRAINER_5"},
    {1073, "EVENT_BEAT_ROUTE_8_TRAINER_0"},
    {1074, "EVENT_BEAT_ROUTE_8_TRAINER_1"},
    {1075, "EVENT_BEAT_ROUTE_8_TRAINER_2"},
    {1076, "EVENT_BEAT_ROUTE_8_TRAINER_3"},
    {1077, "EVENT_BEAT_ROUTE_8_TRAINER_4"},
    {1078, "EVENT_BEAT_ROUTE_8_TRAINER_5"},
    {1079, "EVENT_BEAT_ROUTE_8_TRAINER_6"},
    {1080, "EVENT_BEAT_ROUTE_8_TRAINER_7"},
    {1081, "EVENT_BEAT_ROUTE_8_TRAINER_8"},
    {1089, "EVENT_BEAT_ROUTE_9_TRAINER_0"},
    {1090, "EVENT_BEAT_ROUTE_9_TRAINER_1"},
    {1091, "EVENT_BEAT_ROUTE_9_TRAINER_2"},
    {1092, "EVENT_BEAT_ROUTE_9_TRAINER_3"},
    {1093, "EVENT_BEAT_ROUTE_9_TRAINER_4"},
    {1094, "EVENT_BEAT_ROUTE_9_TRAINER_5"},
    {1095, "EVENT_BEAT_ROUTE_9_TRAINER_6"},
    {1096, "EVENT_BEAT_ROUTE_9_TRAINER_7"},
    {1097, "EVENT_BEAT_ROUTE_9_TRAINER_8"},
    {1105, "EVENT_BEAT_ROUTE_10_TRAINER_0"},
    {1106, "EVENT_BEAT_ROUTE_10_TRAINER_1"},
    {1107, "EVENT_BEAT_ROUTE_10_TRAINER_2"},
    {1108, "EVENT_BEAT_ROUTE_10_TRAINER_3"},
    {1109, "EVENT_BEAT_ROUTE_10_TRAINER_4"},
    {1110, "EVENT_BEAT_ROUTE_10_TRAINER_5"},
    {1113, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_0"},
    {1114, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_1"},
    {1115, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_2"},
    {1116, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_3"},
    {1117, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_4"},
    {1118, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_5"},
    {1119, "EVENT_BEAT_ROCK_TUNNEL_1_TRAINER_6"},
    {1121, "EVENT_BEAT_POWER_PLANT_VOLTORB_0"},
    {1122, "EVENT_BEAT_POWER_PLANT_VOLTORB_1"},
    {1123, "EVENT_BEAT_POWER_PLANT_VOLTORB_2"},
    {1124, "EVENT_BEAT_POWER_PLANT_VOLTORB_3"},
    {1125, "EVENT_BEAT_POWER_PLANT_VOLTORB_4"},
    {1126, "EVENT_BEAT_POWER_PLANT_VOLTORB_5"},
    {1127, "EVENT_BEAT_POWER_PLANT_VOLTORB_6"},
    {1128, "EVENT_BEAT_POWER_PLANT_VOLTORB_7"},
    {1129, "EVENT_BEAT_ZAPDOS"},
    {1137, "EVENT_BEAT_ROUTE_11_TRAINER_0"},
    {1138, "EVENT_BEAT_ROUTE_11_TRAINER_1"},
    {1139, "EVENT_BEAT_ROUTE_11_TRAINER_2"},
    {1140, "EVENT_BEAT_ROUTE_11_TRAINER_3"},
    {1141, "EVENT_BEAT_ROUTE_11_TRAINER_4"},
    {1142, "EVENT_BEAT_ROUTE_11_TRAINER_5"},
    {1143, "EVENT_BEAT_ROUTE_11_TRAINER_6"},
    {1144, "EVENT_BEAT_ROUTE_11_TRAINER_7"},
    {1145, "EVENT_BEAT_ROUTE_11_TRAINER_8"},
    {1146, "EVENT_BEAT_ROUTE_11_TRAINER_9"},
    {1151, "EVENT_GOT_ITEMFINDER"},
    {1152, "EVENT_GOT_TM39"},
    {1154, "EVENT_BEAT_ROUTE_12_TRAINER_0"},
    {1155, "EVENT_BEAT_ROUTE_12_TRAINER_1"},
    {1156, "EVENT_BEAT_ROUTE_12_TRAINER_2"},
    {1157, "EVENT_BEAT_ROUTE_12_TRAINER_3"},
    {1158, "EVENT_BEAT_ROUTE_12_TRAINER_4"},
    {1159, "EVENT_BEAT_ROUTE_12_TRAINER_5"},
    {1160, "EVENT_BEAT_ROUTE_12_TRAINER_6"},
    {1166, "EVENT_FIGHT_ROUTE12_SNORLAX"},
    {1167, "EVENT_BEAT_ROUTE12_SNORLAX"},
    {1169, "EVENT_BEAT_ROUTE_13_TRAINER_0"},
    {1170, "EVENT_BEAT_ROUTE_13_TRAINER_1"},
    {1171, "EVENT_BEAT_ROUTE_13_TRAINER_2"},
    {1172, "EVENT_BEAT_ROUTE_13_TRAINER_3"},
    {1173, "EVENT_BEAT_ROUTE_13_TRAINER_4"},
    {1174, "EVENT_BEAT_ROUTE_13_TRAINER_5"},
    {1175, "EVENT_BEAT_ROUTE_13_TRAINER_6"},
    {1176, "EVENT_BEAT_ROUTE_13_TRAINER_7"},
    {1177, "EVENT_BEAT_ROUTE_13_TRAINER_8"},
    {1178, "EVENT_BEAT_ROUTE_13_TRAINER_9"},
    {1185, "EVENT_BEAT_ROUTE_14_TRAINER_0"},
    {1186, "EVENT_BEAT_ROUTE_14_TRAINER_1"},
    {1187, "EVENT_BEAT_ROUTE_14_TRAINER_2"},
    {1188, "EVENT_BEAT_ROUTE_14_TRAINER_3"},
    {1189, "EVENT_BEAT_ROUTE_14_TRAINER_4"},
    {1190, "EVENT_BEAT_ROUTE_14_TRAINER_5"},
    {1191, "EVENT_BEAT_ROUTE_14_TRAINER_6"},
    {1192, "EVENT_BEAT_ROUTE_14_TRAINER_7"},
    {1193, "EVENT_BEAT_ROUTE_14_TRAINER_8"},
    {1194, "EVENT_BEAT_ROUTE_14_TRAINER_9"},
    {1200, "EVENT_GOT_EXP_ALL"},
    {1201, "EVENT_BEAT_ROUTE_15_TRAINER_0"},
    {1202, "EVENT_BEAT_ROUTE_15_TRAINER_1"},
    {1203, "EVENT_BEAT_ROUTE_15_TRAINER_2"},
    {1204, "EVENT_BEAT_ROUTE_15_TRAINER_3"},
    {1205, "EVENT_BEAT_ROUTE_15_TRAINER_4"},
    {1206, "EVENT_BEAT_ROUTE_15_TRAINER_5"},
    {1207, "EVENT_BEAT_ROUTE_15_TRAINER_6"},
    {1208, "EVENT_BEAT_ROUTE_15_TRAINER_7"},
    {1209, "EVENT_BEAT_ROUTE_15_TRAINER_8"},
    {1210, "EVENT_BEAT_ROUTE_15_TRAINER_9"},
    {1217, "EVENT_BEAT_ROUTE_16_TRAINER_0"},
    {1218, "EVENT_BEAT_ROUTE_16_TRAINER_1"},
    {1219, "EVENT_BEAT_ROUTE_16_TRAINER_2"},
    {1220, "EVENT_BEAT_ROUTE_16_TRAINER_3"},
    {1221, "EVENT_BEAT_ROUTE_16_TRAINER_4"},
    {1222, "EVENT_BEAT_ROUTE_16_TRAINER_5"},
    {1224, "EVENT_FIGHT_ROUTE16_SNORLAX"},
    {1225, "EVENT_BEAT_ROUTE16_SNORLAX"},
    {1230, "EVENT_GOT_HM02"},
    {1231, "EVENT_RESCUED_MR_FUJI"},
    {1233, "EVENT_BEAT_ROUTE_17_TRAINER_0"},
    {1234, "EVENT_BEAT_ROUTE_17_TRAINER_1"},
    {1235, "EVENT_BEAT_ROUTE_17_TRAINER_2"},
    {1236, "EVENT_BEAT_ROUTE_17_TRAINER_3"},
    {1237, "EVENT_BEAT_ROUTE_17_TRAINER_4"},
    {1238, "EVENT_BEAT_ROUTE_17_TRAINER_5"},
    {1239, "EVENT_BEAT_ROUTE_17_TRAINER_6"},
    {1240, "EVENT_BEAT_ROUTE_17_TRAINER_7"},
    {1241, "EVENT_BEAT_ROUTE_17_TRAINER_8"},
    {1242, "EVENT_BEAT_ROUTE_17_TRAINER_9"},
    {1249, "EVENT_BEAT_ROUTE_18_TRAINER_0"},
    {1250, "EVENT_BEAT_ROUTE_18_TRAINER_1"},
    {1251, "EVENT_BEAT_ROUTE_18_TRAINER_2"},
    {1265, "EVENT_BEAT_ROUTE_19_TRAINER_0"},
    {1266, "EVENT_BEAT_ROUTE_19_TRAINER_1"},
    {1267, "EVENT_BEAT_ROUTE_19_TRAINER_2"},
    {1268, "EVENT_BEAT_ROUTE_19_TRAINER_3"},
    {1269, "EVENT_BEAT_ROUTE_19_TRAINER_4"},
    {1270, "EVENT_BEAT_ROUTE_19_TRAINER_5"},
    {1271, "EVENT_BEAT_ROUTE_19_TRAINER_6"},
    {1272, "EVENT_BEAT_ROUTE_19_TRAINER_7"},
    {1273, "EVENT_BEAT_ROUTE_19_TRAINER_8"},
    {1274, "EVENT_BEAT_ROUTE_19_TRAINER_9"},
    {1280, "EVENT_IN_SEAFOAM_ISLANDS"},
    {1281, "EVENT_BEAT_ROUTE_20_TRAINER_0"},
    {1282, "EVENT_BEAT_ROUTE_20_TRAINER_1"},
    {1283, "EVENT_BEAT_ROUTE_20_TRAINER_2"},
    {1284, "EVENT_BEAT_ROUTE_20_TRAINER_3"},
    {1285, "EVENT_BEAT_ROUTE_20_TRAINER_4"},
    {1286, "EVENT_BEAT_ROUTE_20_TRAINER_5"},
    {1287, "EVENT_BEAT_ROUTE_20_TRAINER_6"},
    {1288, "EVENT_BEAT_ROUTE_20_TRAINER_7"},
    {1289, "EVENT_BEAT_ROUTE_20_TRAINER_8"},
    {1290, "EVENT_BEAT_ROUTE_20_TRAINER_9"},
    {1294, "EVENT_SEAFOAM1_BOULDER1_DOWN_HOLE"},
    {1295, "EVENT_SEAFOAM1_BOULDER2_DOWN_HOLE"},
    {1297, "EVENT_BEAT_ROUTE_21_TRAINER_0"},
    {1298, "EVENT_BEAT_ROUTE_21_TRAINER_1"},
    {1299, "EVENT_BEAT_ROUTE_21_TRAINER_2"},
    {1300, "EVENT_BEAT_ROUTE_21_TRAINER_3"},
    {1301, "EVENT_BEAT_ROUTE_21_TRAINER_4"},
    {1302, "EVENT_BEAT_ROUTE_21_TRAINER_5"},
    {1303, "EVENT_BEAT_ROUTE_21_TRAINER_6"},
    {1304, "EVENT_BEAT_ROUTE_21_TRAINER_7"},
    {1305, "EVENT_BEAT_ROUTE_21_TRAINER_8"},
    {1312, "EVENT_1ST_ROUTE22_RIVAL_BATTLE"},
    {1313, "EVENT_2ND_ROUTE22_RIVAL_BATTLE"},
    {1317, "EVENT_BEAT_ROUTE22_RIVAL_1ST_BATTLE"},
    {1318, "EVENT_BEAT_ROUTE22_RIVAL_2ND_BATTLE"},
    {1319, "EVENT_ROUTE22_RIVAL_WANTS_BATTLE"},
    {1328, "EVENT_PASSED_CASCADEBADGE_CHECK"},
    {1329, "EVENT_PASSED_THUNDERBADGE_CHECK"},
    {1330, "EVENT_PASSED_RAINBOWBADGE_CHECK"},
    {1331, "EVENT_PASSED_SOULBADGE_CHECK"},
    {1332, "EVENT_PASSED_MARSHBADGE_CHECK"},
    {1333, "EVENT_PASSED_VOLCANOBADGE_CHECK"},
    {1334, "EVENT_PASSED_EARTHBADGE_CHECK"},
    {1336, "EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH1"},
    {1337, "EVENT_BEAT_VICTORY_ROAD_2_TRAINER_0"},
    {1338, "EVENT_BEAT_VICTORY_ROAD_2_TRAINER_1"},
    {1339, "EVENT_BEAT_VICTORY_ROAD_2_TRAINER_2"},
    {1340, "EVENT_BEAT_VICTORY_ROAD_2_TRAINER_3"},
    {1341, "EVENT_BEAT_VICTORY_ROAD_2_TRAINER_4"},
    {1342, "EVENT_BEAT_MOLTRES"},
    {1343, "EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH2"},
    {1344, "EVENT_GOT_NUGGET"},
    {1345, "EVENT_BEAT_ROUTE24_ROCKET"},
    {1346, "EVENT_BEAT_ROUTE_24_TRAINER_0"},
    {1347, "EVENT_BEAT_ROUTE_24_TRAINER_1"},
    {1348, "EVENT_BEAT_ROUTE_24_TRAINER_2"},
    {1349, "EVENT_BEAT_ROUTE_24_TRAINER_3"},
    {1350, "EVENT_BEAT_ROUTE_24_TRAINER_4"},
    {1351, "EVENT_BEAT_ROUTE_24_TRAINER_5"},
    {1353, "EVENT_NUGGET_REWARD_AVAILABLE"},
    {1360, "EVENT_MET_BILL"},
    {1361, "EVENT_BEAT_ROUTE_25_TRAINER_0"},
    {1362, "EVENT_BEAT_ROUTE_25_TRAINER_1"},
    {1363, "EVENT_BEAT_ROUTE_25_TRAINER_2"},
    {1364, "EVENT_BEAT_ROUTE_25_TRAINER_3"},
    {1365, "EVENT_BEAT_ROUTE_25_TRAINER_4"},
    {1366, "EVENT_BEAT_ROUTE_25_TRAINER_5"},
    {1367, "EVENT_BEAT_ROUTE_25_TRAINER_6"},
    {1368, "EVENT_BEAT_ROUTE_25_TRAINER_7"},
    {1369, "EVENT_BEAT_ROUTE_25_TRAINER_8"},
    {1371, "EVENT_USED_CELL_SEPARATOR_ON_BILL"},
    {1372, "EVENT_GOT_SS_TICKET"},
    {1373, "EVENT_MET_BILL_2"},
    {1374, "EVENT_BILL_SAID_USE_CELL_SEPARATOR"},
    {1375, "EVENT_LEFT_BILLS_HOUSE_AFTER_HELPING"},
    {1378, "EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_0"},
    {1379, "EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_1"},
    {1380, "EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_2"},
    {1393, "EVENT_BEAT_MT_MOON_1_TRAINER_0"},
    {1394, "EVENT_BEAT_MT_MOON_1_TRAINER_1"},
    {1395, "EVENT_BEAT_MT_MOON_1_TRAINER_2"},
    {1396, "EVENT_BEAT_MT_MOON_1_TRAINER_3"},
    {1397, "EVENT_BEAT_MT_MOON_1_TRAINER_4"},
    {1398, "EVENT_BEAT_MT_MOON_1_TRAINER_5"},
    {1399, "EVENT_BEAT_MT_MOON_1_TRAINER_6"},
    {1401, "EVENT_BEAT_MT_MOON_EXIT_SUPER_NERD"},
    {1402, "EVENT_BEAT_MT_MOON_3_TRAINER_0"},
    {1403, "EVENT_BEAT_MT_MOON_3_TRAINER_1"},
    {1404, "EVENT_BEAT_MT_MOON_3_TRAINER_2"},
    {1405, "EVENT_BEAT_MT_MOON_3_TRAINER_3"},
    {1406, "EVENT_GOT_DOME_FOSSIL"},
    {1407, "EVENT_GOT_HELIX_FOSSIL"},
    {1476, "EVENT_BEAT_SS_ANNE_5_TRAINER_0"},
    {1477, "EVENT_BEAT_SS_ANNE_5_TRAINER_1"},
    {1504, "EVENT_GOT_HM01"},
    {1505, "EVENT_RUBBED_CAPTAINS_BACK"},
    {1506, "EVENT_SS_ANNE_LEFT"},
    {1507, "EVENT_WALKED_PAST_GUARD_AFTER_SS_ANNE_LEFT"},
    {1508, "EVENT_STARTED_WALKING_OUT_OF_DOCK"},
    {1509, "EVENT_WALKED_OUT_OF_DOCK"},
    {1521, "EVENT_BEAT_SS_ANNE_8_TRAINER_0"},
    {1522, "EVENT_BEAT_SS_ANNE_8_TRAINER_1"},
    {1523, "EVENT_BEAT_SS_ANNE_8_TRAINER_2"},
    {1524, "EVENT_BEAT_SS_ANNE_8_TRAINER_3"},
    {1537, "EVENT_BEAT_SS_ANNE_9_TRAINER_0"},
    {1538, "EVENT_BEAT_SS_ANNE_9_TRAINER_1"},
    {1539, "EVENT_BEAT_SS_ANNE_9_TRAINER_2"},
    {1540, "EVENT_BEAT_SS_ANNE_9_TRAINER_3"},
    {1553, "EVENT_BEAT_SS_ANNE_10_TRAINER_0"},
    {1554, "EVENT_BEAT_SS_ANNE_10_TRAINER_1"},
    {1555, "EVENT_BEAT_SS_ANNE_10_TRAINER_2"},
    {1556, "EVENT_BEAT_SS_ANNE_10_TRAINER_3"},
    {1557, "EVENT_BEAT_SS_ANNE_10_TRAINER_4"},
    {1558, "EVENT_BEAT_SS_ANNE_10_TRAINER_5"},
    {1632, "EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH1"},
    {1633, "EVENT_BEAT_VICTORY_ROAD_3_TRAINER_0"},
    {1634, "EVENT_BEAT_VICTORY_ROAD_3_TRAINER_1"},
    {1635, "EVENT_BEAT_VICTORY_ROAD_3_TRAINER_2"},
    {1636, "EVENT_BEAT_VICTORY_ROAD_3_TRAINER_3"},
    {1638, "EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH2"},
    {1649, "EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_0"},
    {1650, "EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_1"},
    {1651, "EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_2"},
    {1652, "EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_3"},
    {1653, "EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_4"},
    {1655, "EVENT_ENTERED_ROCKET_HIDEOUT"},
    {1663, "EVENT_67F"},
    {1665, "EVENT_BEAT_ROCKET_HIDEOUT_2_TRAINER_0"},
    {1681, "EVENT_BEAT_ROCKET_HIDEOUT_3_TRAINER_0"},
    {1682, "EVENT_BEAT_ROCKET_HIDEOUT_3_TRAINER_1"},
    {1698, "EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_0"},
    {1699, "EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_1"},
    {1700, "EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_2"},
    {1701, "EVENT_ROCKET_HIDEOUT_4_DOOR_UNLOCKED"},
    {1702, "EVENT_ROCKET_DROPPED_LIFT_KEY"},
    {1703, "EVENT_BEAT_ROCKET_HIDEOUT_GIOVANNI"},
    {1778, "EVENT_BEAT_SILPH_CO_2F_TRAINER_0"},
    {1779, "EVENT_BEAT_SILPH_CO_2F_TRAINER_1"},
    {1780, "EVENT_BEAT_SILPH_CO_2F_TRAINER_2"},
    {1781, "EVENT_BEAT_SILPH_CO_2F_TRAINER_3"},
    {1789, "EVENT_SILPH_CO_2_UNLOCKED_DOOR1"},
    {1790, "EVENT_SILPH_CO_2_UNLOCKED_DOOR2"},
    {1791, "EVENT_GOT_TM36"},
    {1794, "EVENT_BEAT_SILPH_CO_3F_TRAINER_0"},
    {1795, "EVENT_BEAT_SILPH_CO_3F_TRAINER_1"},
    {1800, "EVENT_SILPH_CO_3_UNLOCKED_DOOR1"},
    {1801, "EVENT_SILPH_CO_3_UNLOCKED_DOOR2"},
    {1810, "EVENT_BEAT_SILPH_CO_4F_TRAINER_0"},
    {1811, "EVENT_BEAT_SILPH_CO_4F_TRAINER_1"},
    {1812, "EVENT_BEAT_SILPH_CO_4F_TRAINER_2"},
    {1816, "EVENT_SILPH_CO_4_UNLOCKED_DOOR1"},
    {1817, "EVENT_SILPH_CO_4_UNLOCKED_DOOR2"},
    {1826, "EVENT_BEAT_SILPH_CO_5F_TRAINER_0"},
    {1827, "EVENT_BEAT_SILPH_CO_5F_TRAINER_1"},
    {1828, "EVENT_BEAT_SILPH_CO_5F_TRAINER_2"},
    {1829, "EVENT_BEAT_SILPH_CO_5F_TRAINER_3"},
    {1832, "EVENT_SILPH_CO_5_UNLOCKED_DOOR1"},
    {1833, "EVENT_SILPH_CO_5_UNLOCKED_DOOR2"},
    {1834, "EVENT_SILPH_CO_5_UNLOCKED_DOOR3"},
    {1846, "EVENT_BEAT_SILPH_CO_6F_TRAINER_0"},
    {1847, "EVENT_BEAT_SILPH_CO_6F_TRAINER_1"},
    {1848, "EVENT_BEAT_SILPH_CO_6F_TRAINER_2"},
    {1855, "EVENT_SILPH_CO_6_UNLOCKED_DOOR"},
    {1856, "EVENT_BEAT_SILPH_CO_RIVAL"},
    {1861, "EVENT_BEAT_SILPH_CO_7F_TRAINER_0"},
    {1862, "EVENT_BEAT_SILPH_CO_7F_TRAINER_1"},
    {1863, "EVENT_BEAT_SILPH_CO_7F_TRAINER_2"},
    {1864, "EVENT_BEAT_SILPH_CO_7F_TRAINER_3"},
    {1868, "EVENT_SILPH_CO_7_UNLOCKED_DOOR1"},
    {1869, "EVENT_SILPH_CO_7_UNLOCKED_DOOR2"},
    {1870, "EVENT_SILPH_CO_7_UNLOCKED_DOOR3"},
    {1874, "EVENT_BEAT_SILPH_CO_8F_TRAINER_0"},
    {1875, "EVENT_BEAT_SILPH_CO_8F_TRAINER_1"},
    {1876, "EVENT_BEAT_SILPH_CO_8F_TRAINER_2"},
    {1880, "EVENT_SILPH_CO_8_UNLOCKED_DOOR"},
    {1890, "EVENT_BEAT_SILPH_CO_9F_TRAINER_0"},
    {1891, "EVENT_BEAT_SILPH_CO_9F_TRAINER_1"},
    {1892, "EVENT_BEAT_SILPH_CO_9F_TRAINER_2"},
    {1896, "EVENT_SILPH_CO_9_UNLOCKED_DOOR1"},
    {1897, "EVENT_SILPH_CO_9_UNLOCKED_DOOR2"},
    {1898, "EVENT_SILPH_CO_9_UNLOCKED_DOOR3"},
    {1899, "EVENT_SILPH_CO_9_UNLOCKED_DOOR4"},
    {1905, "EVENT_BEAT_SILPH_CO_10F_TRAINER_0"},
    {1906, "EVENT_BEAT_SILPH_CO_10F_TRAINER_1"},
    {1912, "EVENT_SILPH_CO_10_UNLOCKED_DOOR"},
    {1924, "EVENT_BEAT_SILPH_CO_11F_TRAINER_0"},
    {1925, "EVENT_BEAT_SILPH_CO_11F_TRAINER_1"},
    {1928, "EVENT_SILPH_CO_11_UNLOCKED_DOOR"},
    {1933, "EVENT_GOT_MASTER_BALL"},
    {1935, "EVENT_BEAT_SILPH_CO_GIOVANNI"},
    {2049, "EVENT_BEAT_MANSION_2_TRAINER_0"},
    {2065, "EVENT_BEAT_MANSION_3_TRAINER_0"},
    {2066, "EVENT_BEAT_MANSION_3_TRAINER_1"},
    {2081, "EVENT_BEAT_MANSION_4_TRAINER_0"},
    {2082, "EVENT_BEAT_MANSION_4_TRAINER_1"},
    {2176, "EVENT_GOT_HM03"},
    {2241, "EVENT_BEAT_MEWTWO"},
    {2273, "EVENT_BEAT_LORELEIS_ROOM_TRAINER_0"},
    {2278, "EVENT_AUTOWALKED_INTO_LORELEIS_ROOM"},
    {2281, "EVENT_BEAT_BRUNOS_ROOM_TRAINER_0"},
    {2286, "EVENT_AUTOWALKED_INTO_BRUNOS_ROOM"},
    {2289, "EVENT_BEAT_AGATHAS_ROOM_TRAINER_0"},
    {2294, "EVENT_AUTOWALKED_INTO_AGATHAS_ROOM"},
    {2297, "EVENT_BEAT_LANCES_ROOM_TRAINER_0"},
    {2302, "EVENT_BEAT_LANCE"},
    {2303, "EVENT_LANCES_ROOM_LOCK_DOOR"},
    {2305, "EVENT_BEAT_CHAMPION_RIVAL"},
    {2321, "EVENT_BEAT_VICTORY_ROAD_1_TRAINER_0"},
    {2322, "EVENT_BEAT_VICTORY_ROAD_1_TRAINER_1"},
    {2327, "EVENT_VICTORY_ROAD_1_BOULDER_ON_SWITCH"},
    {2481, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_0"},
    {2482, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_1"},
    {2483, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_2"},
    {2484, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_3"},
    {2485, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_4"},
    {2486, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_5"},
    {2487, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_6"},
    {2488, "EVENT_BEAT_ROCK_TUNNEL_2_TRAINER_7"},
    {2496, "EVENT_SEAFOAM2_BOULDER1_DOWN_HOLE"},
    {2497, "EVENT_SEAFOAM2_BOULDER2_DOWN_HOLE"},
    {2504, "EVENT_SEAFOAM3_BOULDER1_DOWN_HOLE"},
    {2505, "EVENT_SEAFOAM3_BOULDER2_DOWN_HOLE"},
    {2512, "EVENT_SEAFOAM4_BOULDER1_DOWN_HOLE"},
    {2513, "EVENT_SEAFOAM4_BOULDER2_DOWN_HOLE"},
    {2522, "EVENT_BEAT_ARTICUNO"},
};

static bool EventFlagIsSet(const SaveBuffer& buffer, int flagIndex) {
    const int maxFlags = static_cast<int>(Gen1Layout::EventFlagsLen) * 8;
    if (flagIndex < 0 || flagIndex >= maxFlags) {
        return false;
    }

    const std::size_t byteOff = Gen1Layout::EventFlagsOff + static_cast<std::size_t>(flagIndex / 8);
    const u8 bit = static_cast<u8>(flagIndex % 8);
    return buffer.GetBit(byteOff, bit);
}

static bool IsNamedEventSet(const SaveBuffer& buffer, const char* eventName) {
    for (const auto& label : kEventFlagLabels) {
        if (std::string(label.name) == eventName) {
            return EventFlagIsSet(buffer, label.index);
        }
    }
    return false;
}

static bool IsMajorStoryEventName(const std::string& name) {
    static const char* kMajorStoryEvents[] = {
        "EVENT_GOT_STARTER",
        "EVENT_BATTLED_RIVAL_IN_OAKS_LAB",
        "EVENT_GOT_POKEBALLS_FROM_OAK",
        "EVENT_GOT_POKEDEX",
        "EVENT_OAK_GOT_PARCEL",
        "EVENT_GOT_OAKS_PARCEL",
        "EVENT_VIRIDIAN_GYM_OPEN",
        "EVENT_RESCUED_MR_FUJI",
        "EVENT_GOT_POKE_FLUTE",
        "EVENT_MET_BILL",
        "EVENT_GOT_SS_TICKET",
        "EVENT_GOT_HM01",
        "EVENT_SS_ANNE_LEFT",
        "EVENT_GOT_MASTER_BALL",
        "EVENT_BEAT_ROCKET_HIDEOUT_GIOVANNI",
        "EVENT_ROCKET_DROPPED_LIFT_KEY",
        "EVENT_BEAT_SILPH_CO_RIVAL",
        "EVENT_BEAT_SILPH_CO_GIOVANNI",
        "EVENT_GOT_DOME_FOSSIL",
        "EVENT_GOT_HELIX_FOSSIL",
        "EVENT_BEAT_LANCE",
        "EVENT_BEAT_CHAMPION_RIVAL"
    };

    for (const char* major : kMajorStoryEvents) {
        if (name == major) {
            return true;
        }
    }

    return false;
}

static bool IsLegendaryEventName(const std::string& name) {
    return name == "EVENT_BEAT_ARTICUNO" ||
           name == "EVENT_BEAT_ZAPDOS" ||
           name == "EVENT_BEAT_MOLTRES" ||
           name == "EVENT_BEAT_MEWTWO";
}

static bool StartsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

static bool Contains(const std::string& value, const std::string& needle) {
    return value.find(needle) != std::string::npos;
}

static bool HasDigit(const std::string& value) {
    return std::any_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

static std::string TitleCaseToken(const std::string& token) {
    if (token.empty()) {
        return token;
    }

    if (token == "SS") return "S.S.";
    if (token == "MT") return "Mt.";
    if (token == "MR") return "Mr.";
    if (token == "LT") return "Lt.";
    if (token == "CO") return "Co.";
    if (token == "TM") return "TM";
    if (token == "HM") return "HM";
    if (token == "ID") return "ID";
    if (token == "NPC") return "NPC";
    if (token == "PC") return "PC";
    if (token == "PP") return "PP";
    if (token == "HP") return "HP";
    if (token == "B1F" || token == "B2F" || token == "B3F" || token == "B4F") return token;
    if (HasDigit(token)) return token;

    std::string out = token;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    out[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(out[0])));
    return out;
}

static std::string HumanizeEventToken(const std::string& token) {
    if (token == "POKEMONTOWER") return "Pokemon Tower";
    if (token == "POKEMON_TOWER") return "Pokemon Tower";
    if (token == "POWER_PLANT") return "Power Plant";
    if (token == "ROCKET_HIDEOUT") return "Rocket Hideout";
    if (token == "SILPH_CO") return "Silph Co.";
    if (token == "SS_ANNE") return "S.S. Anne";

    std::ostringstream oss;
    std::size_t start = 0;
    bool first = true;
    while (start <= token.size()) {
        const std::size_t sep = token.find('_', start);
        const std::string part = token.substr(start, sep == std::string::npos ? std::string::npos : sep - start);
        if (!part.empty()) {
            if (!first) {
                oss << " ";
            }
            oss << TitleCaseToken(part);
            first = false;
        }
        if (sep == std::string::npos) {
            break;
        }
        start = sep + 1;
    }

    return oss.str();
}

static std::string StripEventPrefix(const std::string& name) {
    if (StartsWith(name, "EVENT_BEAT_")) return name.substr(11);
    if (StartsWith(name, "EVENT_GOT_")) return "GOT_" + name.substr(10);
    if (StartsWith(name, "EVENT_")) return name.substr(6);
    return name;
}

static bool ParseNumberedTrainerEvent(
    const std::string& eventName,
    std::string& locationOut,
    int& trainerNumberOut,
    std::string& labelOut
) {
    constexpr const char* kPrefix = "EVENT_BEAT_";
    constexpr const char* kTrainerMarker = "_TRAINER_";

    if (!StartsWith(eventName, kPrefix)) {
        return false;
    }

    const std::size_t marker = eventName.find(kTrainerMarker);
    if (marker == std::string::npos) {
        return false;
    }

    const std::string locationToken = eventName.substr(std::string(kPrefix).size(), marker - std::string(kPrefix).size());
    const std::size_t numberStart = marker + std::string(kTrainerMarker).size();
    std::size_t numberEnd = numberStart;
    while (numberEnd < eventName.size() && std::isdigit(static_cast<unsigned char>(eventName[numberEnd])) != 0) {
        ++numberEnd;
    }

    if (numberEnd == numberStart) {
        return false;
    }

    const int zeroBased = std::stoi(eventName.substr(numberStart, numberEnd - numberStart));
    trainerNumberOut = zeroBased + 1;
    locationOut = HumanizeEventToken(locationToken);

    std::ostringstream label;
    label << "Trainer #" << trainerNumberOut << ", " << locationOut;
    labelOut = label.str();
    return true;
}

static bool IsStaticEncounterEventName(const std::string& name) {
    return IsLegendaryEventName(name) ||
           name == "EVENT_BEAT_GHOST_MAROWAK" ||
           name == "EVENT_BEAT_ROUTE12_SNORLAX" ||
           name == "EVENT_BEAT_ROUTE16_SNORLAX" ||
           StartsWith(name, "EVENT_BEAT_POWER_PLANT_VOLTORB_");
}

static bool IsTrainerLikeBeatEventName(const std::string& name) {
    return StartsWith(name, "EVENT_BEAT_") && !IsStaticEncounterEventName(name);
}

static std::string TrainerFlagLabel(const std::string& eventName, std::string& locationOut, int& trainerNumberOut) {
    std::string label;
    if (ParseNumberedTrainerEvent(eventName, locationOut, trainerNumberOut, label)) {
        return label;
    }

    struct KnownTrainer {
        const char* eventName;
        const char* label;
        const char* location;
    };

    static const KnownTrainer kKnownTrainers[] = {
        {"EVENT_BEAT_BROCK", "Gym Leader Brock, Pewter Gym", "Pewter Gym"},
        {"EVENT_BEAT_MISTY", "Gym Leader Misty, Cerulean Gym", "Cerulean Gym"},
        {"EVENT_BEAT_LT_SURGE", "Gym Leader Lt. Surge, Vermilion Gym", "Vermilion Gym"},
        {"EVENT_BEAT_ERIKA", "Gym Leader Erika, Celadon Gym", "Celadon Gym"},
        {"EVENT_BEAT_KOGA", "Gym Leader Koga, Fuchsia Gym", "Fuchsia Gym"},
        {"EVENT_BEAT_SABRINA", "Gym Leader Sabrina, Saffron Gym", "Saffron Gym"},
        {"EVENT_BEAT_BLAINE", "Gym Leader Blaine, Cinnabar Gym", "Cinnabar Gym"},
        {"EVENT_BEAT_VIRIDIAN_GYM_GIOVANNI", "Gym Leader Giovanni, Viridian Gym", "Viridian Gym"},
        {"EVENT_BEAT_ROCKET_HIDEOUT_GIOVANNI", "Giovanni, Rocket Hideout", "Rocket Hideout"},
        {"EVENT_BEAT_SILPH_CO_RIVAL", "Rival, Silph Co.", "Silph Co."},
        {"EVENT_BEAT_SILPH_CO_GIOVANNI", "Giovanni, Silph Co.", "Silph Co."},
        {"EVENT_BEAT_LANCE", "Elite Four Lance", "Lance's Room"},
        {"EVENT_BEAT_CHAMPION_RIVAL", "Champion Rival", "Indigo Plateau"},
    };

    for (const auto& known : kKnownTrainers) {
        if (eventName == known.eventName) {
            locationOut = known.location;
            trainerNumberOut = 0;
            return known.label;
        }
    }

    locationOut = HumanizeEventToken(StripEventPrefix(eventName));
    trainerNumberOut = 0;
    return "Named battle, " + locationOut;
}

static std::string StoryCategoryForEventName(const std::string& name) {
    if (IsStaticEncounterEventName(name)) return "static_battle";
    if (IsMajorStoryEventName(name)) return "major_story";
    if (StartsWith(name, "EVENT_GOT_")) return "item_or_gift";
    if (Contains(name, "BADGE") || Contains(name, "GYM")) return "gym_or_badge";
    if (Contains(name, "UNLOCK") || Contains(name, "LOCK") || Contains(name, "DOOR") || Contains(name, "OPEN")) {
        return "door_or_unlock";
    }
    if (Contains(name, "BOULDER") || Contains(name, "SWITCH") || Contains(name, "HOLE")) {
        return "environment_puzzle";
    }
    if (Contains(name, "ROCKET") || Contains(name, "SILPH") || Contains(name, "RIVAL") ||
        Contains(name, "FUJI") || Contains(name, "BILL") || Contains(name, "OAK") ||
        Contains(name, "SS_ANNE") || Contains(name, "SAFARI")) {
        return "story_progress";
    }
    return "misc_event";
}

static std::string StoryFlagLabel(const std::string& eventName) {
    return HumanizeEventToken(StripEventPrefix(eventName));
}

static int CountUsedBits(
    const SaveBuffer& buffer,
    std::size_t byteOff,
    int usedBits,
    std::vector<int>* firstSet,
    std::size_t firstSetLimit = 10
) {
    if (usedBits <= 0) {
        return 0;
    }

    buffer.RequireRange(byteOff, static_cast<std::size_t>((usedBits + 7) / 8));

    int total = 0;
    for (int bitIndex = 0; bitIndex < usedBits; ++bitIndex) {
        const std::size_t off = byteOff + static_cast<std::size_t>(bitIndex / 8);
        const u8 bit = static_cast<u8>(bitIndex % 8);
        if (buffer.GetBit(off, bit)) {
            ++total;
            if (firstSet != nullptr && firstSet->size() < firstSetLimit) {
                firstSet->push_back(bitIndex);
            }
        }
    }

    return total;
}

} // namespace

// Event flag summary:
// Bulbapedia lists a large completed-game-events bitfield (0x29F3, length 0x140).
// For MVP, we count set bits and list indices.
FlagSummary ReadOnlyData::GetEventFlagSummary() const {
    FlagSummary out;

    buffer_.RequireRange(Gen1Layout::EventFlagsOff, Gen1Layout::EventFlagsLen);

    int totalChecked = static_cast<int>(Gen1Layout::EventFlagsLen) * 8;
    int totalSet = 0;

    for (std::size_t i = 0; i < Gen1Layout::EventFlagsLen; ++i) {
        const u8 b = buffer_.ReadU8(Gen1Layout::EventFlagsOff + i);
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
    out.namedFlagsKnown = static_cast<int>(sizeof(kEventFlagLabels) / sizeof(kEventFlagLabels[0]));

    for (const auto& label : kEventFlagLabels) {
        if (label.index < 0 || label.index >= totalChecked) {
            continue;
        }

        const std::size_t byteIndex = static_cast<std::size_t>(label.index / 8);
        const int bitIndex = label.index % 8;
        const u8 byte = buffer_.ReadU8(Gen1Layout::EventFlagsOff + byteIndex);
        const bool isSet = (byte & static_cast<u8>(1u << bitIndex)) != 0;

        FlagSummary::NamedFlag named;
        named.index = label.index;
        named.name = label.name;
        named.isSet = isSet;
        out.namedFlags.push_back(named);

        if (isSet) {
            out.namedSetFlags.push_back(std::move(named));
            out.namedFlagsSet++;
            if (IsBeatEventName(label.name)) {
                out.beatFlagsSet++;
            }
        }
    }

    return out;
}

EventCategorySummary ReadOnlyData::GetEventCategorySummary() const {
    EventCategorySummary out;
    const FlagSummary flags = GetEventFlagSummary();
    const TrainerSummary trainer = GetTrainerSummary();

    for (const auto& named : flags.namedFlags) {
        if (IsTrainerLikeBeatEventName(named.name)) {
            EventCategorySummary::TrainerFlag trainerFlag;
            trainerFlag.index = named.index;
            trainerFlag.eventName = named.name;
            trainerFlag.isComplete = named.isSet;
            trainerFlag.label = TrainerFlagLabel(named.name, trainerFlag.location, trainerFlag.trainerNumber);

            out.trainerFlags.push_back(std::move(trainerFlag));
            out.trainerFlagsKnown++;
            if (named.isSet) {
                out.trainerFlagsComplete++;
                out.defeatedTrainerFlagsSet++;
                out.defeatedTrainerFlags.push_back(named.name);
            }
        }

        if (IsMajorStoryEventName(named.name)) {
            if (named.isSet) {
                out.majorStoryMilestones.push_back(named.name);
                out.majorStoryMilestonesSet++;
            }
        }

        if (IsLegendaryEventName(named.name)) {
            if (named.isSet) {
                out.legendaryFlags.push_back(named.name);
                out.legendaryFlagsSet++;
            }
        }

        if (IsStaticEncounterEventName(named.name)) {
            EventCategorySummary::StoryFlag staticFlag;
            staticFlag.index = named.index;
            staticFlag.eventName = named.name;
            staticFlag.label = StoryFlagLabel(named.name);
            staticFlag.category = StoryCategoryForEventName(named.name);
            staticFlag.isComplete = named.isSet;
            out.staticEncounterFlags.push_back(std::move(staticFlag));
            out.staticEncounterFlagsKnown++;
            if (named.isSet) {
                out.staticEncounterFlagsComplete++;
            }
        }

        const bool storyLike = !IsTrainerLikeBeatEventName(named.name) || IsMajorStoryEventName(named.name);
        if (storyLike) {
            EventCategorySummary::StoryFlag storyFlag;
            storyFlag.index = named.index;
            storyFlag.eventName = named.name;
            storyFlag.label = StoryFlagLabel(named.name);
            storyFlag.category = StoryCategoryForEventName(named.name);
            storyFlag.isComplete = named.isSet;
            out.storyFlags.push_back(std::move(storyFlag));
            out.storyFlagsKnown++;
            if (named.isSet) {
                out.storyFlagsComplete++;
            }
        }
    }

    struct GymMap {
        const char* badgeName;
        const char* eventName;
        int badgeBit;
    };

    static const GymMap kGyms[] = {
        {"Boulder Badge", "EVENT_BEAT_BROCK", 0},
        {"Cascade Badge", "EVENT_BEAT_MISTY", 1},
        {"Thunder Badge", "EVENT_BEAT_LT_SURGE", 2},
        {"Rainbow Badge", "EVENT_BEAT_ERIKA", 3},
        {"Soul Badge", "EVENT_BEAT_KOGA", 4},
        {"Marsh Badge", "EVENT_BEAT_SABRINA", 5},
        {"Volcano Badge", "EVENT_BEAT_BLAINE", 6},
        {"Earth Badge", "EVENT_BEAT_VIRIDIAN_GYM_GIOVANNI", 7},
    };

    for (const auto& gym : kGyms) {
        EventCategorySummary::GymConsistency row;
        row.badgeName = gym.badgeName;
        row.eventName = gym.eventName;
        row.badgeOwned = (trainer.badges & static_cast<u8>(1u << gym.badgeBit)) != 0;
        row.leaderEventSet = IsNamedEventSet(buffer_, gym.eventName);
        row.consistent = (row.badgeOwned == row.leaderEventSet);

        if (row.leaderEventSet) {
            out.gymLeaderFlagsSet++;
        }
        if (!row.consistent) {
            out.gymStoryMismatchCount++;
        }

        out.gymConsistency.push_back(std::move(row));
    }

    return out;
}

PlayerStateSummary ReadOnlyData::GetPlayerStateSummary() const {
    PlayerStateSummary out;

    out.optionsByte = buffer_.ReadU8(Gen1Layout::OptionsOff);
    out.letterDelayByte = buffer_.ReadU8(Gen1Layout::LetterDelayOff);
    out.contrast = buffer_.ReadU8(Gen1Layout::ContrastOff);

    out.yBlockCoord = buffer_.ReadU8(Gen1Layout::YBlockCoordOff);
    out.xBlockCoord = buffer_.ReadU8(Gen1Layout::XBlockCoordOff);
    out.specialWarpY = buffer_.ReadU8(Gen1Layout::SpecialWarpYOff);
    out.specialWarpX = buffer_.ReadU8(Gen1Layout::SpecialWarpXOff);
    out.playerMoveDir = buffer_.ReadU8(Gen1Layout::PlayerMoveDirOff);
    out.playerLastStopDir = buffer_.ReadU8(Gen1Layout::PlayerLastStopDirOff);
    out.playerCurDir = buffer_.ReadU8(Gen1Layout::PlayerCurDirOff);
    out.walkBikeSurf = buffer_.ReadU8(Gen1Layout::WalkBikeSurfOff);
    out.playerJumpingYScreen = buffer_.ReadU8(Gen1Layout::PlayerJumpingYScreenOff);

    out.safariSteps = ReadU16BE_At(buffer_, Gen1Layout::SafariStepsOff);
    out.safariGameOver = buffer_.ReadU8(Gen1Layout::SafariGameOverOff) == 1;
    out.safariBallCount = buffer_.ReadU8(Gen1Layout::SafariBallCountOff);

    out.strengthOutsideBattle = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 0);
    out.surfingAllowed = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 1);
    out.usedCardKey = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 7);

    out.isBattle = buffer_.GetBit(Gen1Layout::BattleFlagsOff, 6);
    out.isTrainerBattle = buffer_.GetBit(Gen1Layout::BattleFlagsOff, 7);
    out.noBattles = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 4);
    out.battleEndedOrBlackout = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 5);
    out.usingLinkCable = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 6);
    out.flyOutOfBattle = buffer_.GetBit(Gen1Layout::FlyFlagsOff, 7);

    out.standingOnDoor = buffer_.GetBit(Gen1Layout::DoorWarpFlagsOff, 0);
    out.movingThroughDoor = buffer_.GetBit(Gen1Layout::DoorWarpFlagsOff, 1);
    out.standingOnWarp = buffer_.GetBit(Gen1Layout::DoorWarpFlagsOff, 2);
    out.finalLedgeJumping = buffer_.GetBit(Gen1Layout::DoorWarpFlagsOff, 6);
    out.spinPlayer = buffer_.GetBit(Gen1Layout::DoorWarpFlagsOff, 7);

    out.noLetterDelay = buffer_.GetBit(Gen1Layout::TextFlagsOff, 6);
    out.countPlaytime = buffer_.GetBit(Gen1Layout::PlaytimeFlagsOff, 0);

    return out;
}

WorldStateSummary ReadOnlyData::GetWorldStateSummary() const {
    WorldStateSummary out;

    out.missableObjectsChecked = Gen1Layout::MissableObjectsUsedBits;
    out.missableObjectsSet = CountUsedBits(
        buffer_,
        Gen1Layout::MissableObjectsOff,
        Gen1Layout::MissableObjectsUsedBits,
        &out.firstSetMissableObjects
    );

    out.hiddenItemsChecked = Gen1Layout::HiddenItemsUsedBits;
    out.hiddenItemsCollected = CountUsedBits(
        buffer_,
        Gen1Layout::HiddenItemsOff,
        Gen1Layout::HiddenItemsUsedBits,
        &out.firstCollectedHiddenItems
    );

    out.hiddenCoinsChecked = Gen1Layout::HiddenCoinsUsedBits;
    out.hiddenCoinsCollected = CountUsedBits(
        buffer_,
        Gen1Layout::HiddenCoinsOff,
        Gen1Layout::HiddenCoinsUsedBits,
        &out.firstCollectedHiddenCoins
    );

    out.visitedTownsChecked = Gen1Layout::VisitedTownsUsedBits;
    out.visitedTownsSet = CountUsedBits(
        buffer_,
        Gen1Layout::VisitedTownsOff,
        Gen1Layout::VisitedTownsUsedBits,
        &out.visitedTownIndices,
        Gen1Layout::VisitedTownsUsedBits
    );

    buffer_.RequireRange(Gen1Layout::CurrentScriptsOff, Gen1Layout::CurrentScriptsLen);
    out.currentScriptsChecked = static_cast<int>(Gen1Layout::CurrentScriptsLen);
    for (std::size_t i = 0; i < Gen1Layout::CurrentScriptsLen; ++i) {
        if (buffer_.ReadU8(Gen1Layout::CurrentScriptsOff + i) != 0) {
            out.currentScriptsNonZero++;
            if (out.nonZeroCurrentScriptIndices.size() < 10) {
                out.nonZeroCurrentScriptIndices.push_back(static_cast<int>(i));
            }
        }
    }

    out.gotOldRod = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 3);
    out.gotGoodRod = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 4);
    out.gotSuperRod = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 5);
    out.satisfiedSaffronGuards = buffer_.GetBit(Gen1Layout::WorldFlags1Off, 6);

    out.gotLapras = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 0);
    out.everHealedPokemon = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 2);
    out.gotStarter = buffer_.GetBit(Gen1Layout::WorldFlags2Off, 3);
    out.defeatedLorelei = buffer_.GetBit(Gen1Layout::EliteFlagsOff, 1);

    return out;
}

DaycareSummary ReadOnlyData::GetDaycareSummary() const {
    DaycareSummary out;
    out.inUse = buffer_.ReadU8(Gen1Layout::DaycareInUseOff) > 0;

    if (!out.inUse) {
        return out;
    }

    PokemonMon mon;
    mon.position = 1;
    mon.nickname = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::DaycareNicknameOff, Gen1Layout::Gen1NameLen);
    mon.otName = Gen1TextCodec::DecodeName(buffer_, Gen1Layout::DaycareOTNameOff, Gen1Layout::Gen1NameLen);
    DecodeBoxMonStruct(buffer_, Gen1Layout::DaycareBoxMonOff, mon);
    mon.speciesName = Gen1SpeciesLookup::NameFromId(mon.speciesId);
    mon.dexNo = Gen1SpeciesLookup::DexfromId(mon.speciesId);

    out.pokemon = std::move(mon);
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
            mon.speciesName = Gen1SpeciesLookup::NameFromId(static_cast<u8>(species));
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

    oss << "--- Player / Options State ---\n";
    oss << GetPlayerStateSummary().ToString() << "\n";

    // Checksums
    oss << "Main Checksum: " << (Gen1Checksum::ValidateMain(buffer_) ? "VALID" : "INVALID") << "\n";
    oss << "Bank2 All Checksum: " << (Gen1Checksum::ValidateBankAll(buffer_, 2) ? "VALID" : "INVALID") << "\n";
    oss << "Bank3 All Checksum: " << (Gen1Checksum::ValidateBankAll(buffer_, 3) ? "VALID" : "INVALID") << "\n";

    // Party
    oss << "\n--- Pokemon Party Decode (Box 0) ---\n";
    oss << GetPartyAsBox0().ToString() << "\n";
    
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
    
    oss << "\nNOTE: Gen I also stores a \"Current Box\" cache in Bank 1 (0x30C0..0x3521).\n"
           "Boxes 1–12 are stored in Banks 2/3, but the currently selected box may appear updated in the Bank 1 cache.\n"
           "So if something looks \"missing\" or \"outdated\" here, it may be because you are viewing the cached copy vs. the bank copy.\n\n";

    oss << "--- Current Box Cache ---\n";
    const PokemonBox currentBoxCache = GetCurrentBoxCache();
    oss << currentBoxCache.ToString() << "\n";
    
    
    oss << "--- Bag ---\n";
    const BagSummary bag = GetBagSummary(true);
    oss << bag.ToString() << "\n";

    oss << "--- PC Item Box ---\n";
    const BagSummary pcBox = GetPCItemBoxSummary(true);
    oss << pcBox.ToString() << "\n";

    oss << "--- Daycare ---\n";
    oss << GetDaycareSummary().ToString() << "\n";
    
    
    // Event flags
    oss << "--- Event Flags (Summary) ---\n";
    const FlagSummary fs = GetEventFlagSummary();
    oss << fs.ToString() << "\n";

    oss << "--- Event / Story Categories ---\n";
    oss << GetEventCategorySummary().ToString() << "\n";

    oss << "--- World / Object State ---\n";
    oss << GetWorldStateSummary().ToString() << "\n";

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
