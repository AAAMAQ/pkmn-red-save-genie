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

    PokemonBox box;
    box.boxNumber = boxIndex1to12;
    box.label = std::string("PC Box ") + std::to_string(boxIndex1to12);

    const std::size_t base = Gen1Layout::BoxBaseOffsetByIndex1to12(boxIndex1to12);
    buffer_.RequireRange(base, Gen1Layout::BoxBlockSize);

    const int rawCount = static_cast<int>(buffer_.ReadU8(base + Gen1Layout::BoxCountRel));
    box.pokemonCount = std::clamp(rawCount, 0, Gen1Layout::BoxMaxMons);

    for (int i = 0; i < box.pokemonCount; ++i) {
        PokemonMon mon;
        mon.position = i + 1;

        // Species list in box block.
        const u8 speciesId = buffer_.ReadU8(base + Gen1Layout::BoxSpeciesRel + static_cast<std::size_t>(i));
        mon.speciesId = speciesId;
        mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
        mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);

        // OT and nickname arrays
        mon.otName = Gen1TextCodec::DecodeName(buffer_, base + Gen1Layout::BoxOTNamesRel + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);
        mon.nickname = Gen1TextCodec::DecodeName(buffer_, base + Gen1Layout::BoxNicknamesRel + static_cast<std::size_t>(i) * Gen1Layout::Gen1NameLen, Gen1Layout::Gen1NameLen);

        // Box struct
        const std::size_t monStructOff = base + Gen1Layout::BoxStructsRel + static_cast<std::size_t>(i) * Gen1Layout::BoxStructSize;
        DecodeBoxMonStruct(buffer_, monStructOff, mon);

        // If struct species is 0, keep list species.
        if (mon.speciesId == 0) {
            mon.speciesId = speciesId;
            mon.speciesName = Gen1SpeciesLookup::NameFromId(speciesId);
            mon.dexNo = Gen1SpeciesLookup::DexfromId(speciesId);
        }

        box.pokemon.push_back(std::move(mon));
    }

    return box;
}

PokemonBoxesExport ReadOnlyData::GetAllBoxesExport() const {
    PokemonBoxesExport out;
    out.boxes.reserve(13);

    out.boxes.push_back(GetPartyAsBox0());
    for (int i = 1; i <= 12; ++i) {
        out.boxes.push_back(GetPCBox(i));
    }

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
    
    
    oss << "--- Bag ---\n";
    const BagSummary bag = GetBagSummary(true);
    oss << bag.ToString() << "\n";

    oss << "--- PC Item Box ---\n";
    const BagSummary pcBox = GetPCItemBoxSummary(true);
    oss << pcBox.ToString() << "\n";
    
    
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

