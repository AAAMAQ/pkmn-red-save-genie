//
//  SaveStructure.cpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Implementation of SaveStructure.hpp.
//   - This file is intentionally FILE-I/O FREE.
//   - It operates on an in-memory byte buffer only.
//

#include "SaveStructure.hpp"

#include <algorithm>
#include <sstream>

namespace savegenie {

// =========================================================
// SaveBuffer
// =========================================================

SaveBuffer::SaveBuffer() : bytes_() {}

SaveBuffer::SaveBuffer(Bytes bytes) : bytes_(std::move(bytes)) {}

std::size_t SaveBuffer::Size() const { return bytes_.size(); }

const SaveBuffer::Bytes& SaveBuffer::BytesView() const { return bytes_; }

SaveBuffer::Bytes& SaveBuffer::BytesMutable() { return bytes_; }

void SaveBuffer::RequireRange(std::size_t off, std::size_t len) const {
    if (len == 0) return;

    // Protect against overflow in off + len
    if (off > bytes_.size()) {
        throw std::out_of_range("SaveBuffer: offset out of range");
    }
    const std::size_t end = off + len;
    if (end < off || end > bytes_.size()) {
        throw std::out_of_range("SaveBuffer: range out of range");
    }
}

u8 SaveBuffer::ReadU8(std::size_t off) const {
    RequireRange(off, 1);
    return bytes_[off];
}

u16 SaveBuffer::ReadU16LE(std::size_t off) const {
    RequireRange(off, 2);
    return static_cast<u16>(bytes_[off]) |
           static_cast<u16>(static_cast<u16>(bytes_[off + 1]) << 8);
}

u32 SaveBuffer::ReadU24BE(std::size_t off) const {
    RequireRange(off, 3);
    return (static_cast<u32>(bytes_[off]) << 16) |
           (static_cast<u32>(bytes_[off + 1]) << 8) |
           (static_cast<u32>(bytes_[off + 2]));
}

void SaveBuffer::WriteU8(std::size_t off, u8 v) {
    RequireRange(off, 1);
    bytes_[off] = v;
}

void SaveBuffer::WriteU16LE(std::size_t off, u16 v) {
    RequireRange(off, 2);
    bytes_[off]     = static_cast<u8>(v & 0xFF);
    bytes_[off + 1] = static_cast<u8>((v >> 8) & 0xFF);
}

void SaveBuffer::WriteU24BE(std::size_t off, u32 v) {
    RequireRange(off, 3);
    bytes_[off]     = static_cast<u8>((v >> 16) & 0xFF);
    bytes_[off + 1] = static_cast<u8>((v >> 8) & 0xFF);
    bytes_[off + 2] = static_cast<u8>(v & 0xFF);
}

bool SaveBuffer::GetBit(std::size_t byteOff, u8 bitIndex0to7) const {
    if (bitIndex0to7 >= 8) {
        throw std::out_of_range("SaveBuffer: bitIndex must be 0..7");
    }
    RequireRange(byteOff, 1);
    const u8 mask = static_cast<u8>(1u << bitIndex0to7);
    return (bytes_[byteOff] & mask) != 0;
}

void SaveBuffer::SetBit(std::size_t byteOff, u8 bitIndex0to7, bool value) {
    if (bitIndex0to7 >= 8) {
        throw std::out_of_range("SaveBuffer: bitIndex must be 0..7");
    }
    RequireRange(byteOff, 1);
    const u8 mask = static_cast<u8>(1u << bitIndex0to7);
    if (value) {
        bytes_[byteOff] |= mask;
    } else {
        bytes_[byteOff] &= static_cast<u8>(~mask);
    }
}

SaveBuffer::Bytes SaveBuffer::Slice(std::size_t off, std::size_t len) const {
    RequireRange(off, len);
    return Bytes(bytes_.begin() + static_cast<std::ptrdiff_t>(off),
                 bytes_.begin() + static_cast<std::ptrdiff_t>(off + len));
}

// =========================================================
// Gen1Layout helpers
// =========================================================

std::size_t Gen1Layout::BoxBaseOffsetByIndex1to12(int boxIndex1to12) {
    if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
        throw std::out_of_range("Gen1Layout: box index must be 1..12");
    }

    // Bank 2: boxes 1..6, Bank 3: boxes 7..12
    if (boxIndex1to12 <= 6) {
        return Box1Off + static_cast<std::size_t>(boxIndex1to12 - 1) * BoxBlockSize;
    }
    return Box7Off + static_cast<std::size_t>(boxIndex1to12 - 7) * BoxBlockSize;
}

std::size_t Gen1Layout::BankAllChecksumOffsetForBoxIndex1to12(int boxIndex1to12) {
    if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
        throw std::out_of_range("Gen1Layout: box index must be 1..12");
    }
    return (boxIndex1to12 <= 6) ? Bank2AllChecksumOff : Bank3AllChecksumOff;
}

std::size_t Gen1Layout::BankPerBoxChecksumsBaseOffsetForBoxIndex1to12(int boxIndex1to12) {
    if (boxIndex1to12 < 1 || boxIndex1to12 > 12) {
        throw std::out_of_range("Gen1Layout: box index must be 1..12");
    }
    return (boxIndex1to12 <= 6) ? Bank2BoxChecksumsOff : Bank3BoxChecksumsOff;
}

// =========================================================
// Gen1MapLookup (fixed-size arrays)
// =========================================================

// Human-readable map names
const std::string Gen1MapLookup::MapIDName[256] = {
    
          "Pallet Town",
          "Viridian City",
          "Pewter City",
          "Cerulean City",
          "Lavender Town",
          "Vermilion City",
          "Celadon City",
          "Fuchsia City",
          "Cinnabar Island",
          "Pokémon League",
          "Saffron City",
          "INVALID",
          "Route 1",
          "Route 2",
          "Route 3",
          "Route 4",
          "Route 5",
          "Route 6",
          "Route 7",
          "Route 8",
          "Route 9",
          "Route 10",
          "Route 11",
          "Route 12",
          "Route 13",
          "Route 14",
          "Route 15",
          "Route 16",
          "Route 17",
          "Route 18",
          "Sea Route 19",
          "Sea Route 20",
          "Sea Route 21",
          "Route 22",
          "Route 23",
          "Route 24",
          "Route 25",
          "Red's house (first floor)",
          "Red's house (second floor)",
          "Blue's house",
          "Professor Oak's Lab",
          "Pokémon Center (Viridian City)",
          "Poké Mart (Viridian City)",
          "School (Viridian City)",
          "House 1 (Viridian City)",
          "Gym (Viridian City)",
          "Diglett's Cave (Route 2 entrance)",
          "Gate (Viridian City/Pewter City) (Route 2)",
          "Oak's Aide House 1 (Route 2)",
          "Gate (Route 2)",
          "Gate (Route 2/Viridian Forest) (Route 2)",
          "Viridian Forest",
          "Pewter Museum (floor 1)",
          "Pewter Museum (floor 2)",
          "Gym (Pewter City)",
          "House with disobedient Nidoran♂ (Pewter City)",
          "Poké Mart (Pewter City)",
          "House with two Trainers (Pewter City)",
          "Pokémon Center (Pewter City)",
          "Mt. Moon (Route 3 entrance)",
          "Mt. Moon",
          "Mt. Moon",
          "Invaded house (Cerulean City)",
          "Poliwhirl for Jynx trade house (Red/Blue)",
          "Pokémon Center (Cerulean City)",
          "Gym (Cerulean City)",
          "Bike Shop (Cerulean City)",
          "Poké Mart (Cerulean City)",
          "Pokémon Center (Route 4)",
          "Invaded house - alternative music (Cerulean City)",
          "Saffron City Gate (Route 5)",
          "Entrance to Underground Path (Route 5)",
          "Daycare Center (Route 5)",
          "Saffron City Gate (Route 6)",
          "Entrance to Underground Path (Route 6)",
          "Entrance to Underground Path (alternative music) (Route 6)",
          "Saffron City Gate (Route 7)",
          "Entrance to Underground Path (Route 7)",
          "INVALID",
          "Saffron City Gate (Route 8)",
          "Entrance to Underground Path (Route 8)",
          "Pokémon Center (Rock Tunnel)",
          "Rock Tunnel",
          "Power Plant",
          "Gate 1F (Route 11-Route 12)",
          "Diglett's Cave (Vermilion City entrance)",
          "Gate 2F (Route 11-Route 12)",
          "Gate (Route 12-Route 13)",
          "Sea Cottage",
          "Pokémon Center (Vermilion City)",
          "Pokémon Fan Club (Vermilion City)",
          "Poké Mart (Vermilion City)",
          "Gym (Vermilion City)",
          "House with Pidgey (Vermilion City)",
          "Vermilion Harbor (Vermilion City)",
          "S.S. Anne 1F",
          "S.S. Anne 2F",
          "S.S. Anne 3F",
          "S.S. Anne B1F",
          "S.S. Anne (Deck)",
          "S.S. Anne (Kitchen)",
          "S.S. Anne (Captain's room)",
          "S.S. Anne 1F (Gentleman's room)",
          "S.S. Anne 2F (Gentleman's room)",
          "S.S. Anne B1F (Sailor/Fisherman's room)",
          "INVALID",
          "INVALID",
          "INVALID",
          "Victory Road (Route 23 entrance)",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "Lance's Elite Four room",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "Hall of Fame",
          "Underground Path (Route 5-Route 6)",
          "Blue (Champion)'s room",
          "Underground Path (Route 7-Route 8)",
          "Celadon Department Store 1F",
          "Celadon Department Store 2F",
          "Celadon Department Store 3F",
          "Celadon Department Store 4F",
          "Celadon Department Store Rooftop Square",
          "Celadon Department Store Lift",
          "Celadon Mansion 1F",
          "Celadon Mansion 2F",
          "Celadon Mansion 3F",
          "Celadon Mansion 4F",
          "Celadon Mansion 4F (Eevee building)",
          "Pokémon Center (Celadon City)",
          "Gym (Celadon City)",
          "Rocket Game Corner (Celadon City)",
          "Celadon Department Store 5F",
          "Prize corner (Celadon City)",
          "Restaurant (Celadon City)",
          "House with Team Rocket members (Celadon City)",
          "Hotel (Celadon City)",
          "Pokémon Center (Lavender Town)",
          "Pokémon Tower 1F",
          "Pokémon Tower 2F",
          "Pokémon Tower 3F",
          "Pokémon Tower 4F",
          "Pokémon Tower 5F",
          "Pokémon Tower 6F",
          "Pokémon Tower 7F",
          "Mr. Fuji's house (Lavender Town)",
          "Poké Mart (Lavender Town)",
          "House with NPC discussing Cubone's mother",
          "Poké Mart (Fuchsia City)",
          "House with NPCs discussing Bill (Fuchsia City)",
          "Pokémon Center (Fuchsia City)",
          "Warden's house (Fuchsia City)",
          "Safari Zone gate (Fuchsia City)",
          "Gym (Fuchsia City)",
          "House with NPCs discussing Baoba (Fuchsia City)",
          "Seafoam Islands",
          "Seafoam Islands",
          "Seafoam Islands",
          "Seafoam Islands",
          "Vermilion City Fishing Brother",
          "Fuchsia City Fishing Brother",
          "Pokémon Mansion (1F)",
          "Gym (Cinnabar Island)",
          "Pokémon Lab (Cinnabar Island)",
          "Pokémon Lab - Trade room (Cinnabar Island)",
          "Pokémon Lab - Room with scientists (Cinnabar Island)",
          "Pokémon Lab - Fossil resurrection room (Cinnabar Island)",
          "Pokémon Center (Cinnabar Island)",
          "Poké Mart (Cinnabar Island)",
          "Poké Mart - alternative music (Cinnabar Island)",
          "Pokémon Center (Indigo Plateau)",
          "Copycat's house 1F (Saffron City)",
          "Copycat's house 2F (Saffron City)",
          "Fighting Dojo (Saffron City)",
          "Gym (Saffron City)",
          "House with Pidgey (Saffron City)",
          "Poké Mart (Saffron City)",
          "Silph Co. 1F",
          "Pokémon Center (Saffron City)",
          "Mr. Psychic's house (Saffron City)",
          "Gate 1F (Route 15)",
          "Gate 2F (Route 15)",
          "Gate 1F (Cycling Road) (Route 16)",
          "Gate 2F (Cycling Road) (Route 16)",
          "Secret house (Cycling Road) (Route 16)",
          "Route 12 Fishing Brother",
          "Gate 1F (Route 18)",
          "Gate 2F (Route 18)",
          "Seafoam Islands",
          "Badges check gate (Route 22)",
          "Victory Road",
          "Gate 2F (Route 12)",
          "House with NPC and HM moves advice (Vermilion City)",
          "Diglett's Cave",
          "Victory Road",
          "Team Rocket Hideout (B1F)",
          "Team Rocket Hideout (B2F)",
          "Team Rocket Hideout (B3F)",
          "Team Rocket Hideout (B4F)",
          "Team Rocket Hideout (Lift)",
          "INVALID",
          "INVALID",
          "INVALID",
          "Silph Co. (2F)",
          "Silph Co. (3F)",
          "Silph Co. (4F)",
          "Silph Co. (5F)",
          "Silph Co. (6F)",
          "Silph Co. (7F)",
          "Silph Co. (8F)",
          "Pokémon Mansion (2F)",
          "Pokémon Mansion (3F)",
          "Pokémon Mansion (B1F)",
          "Safari Zone (Area 1)",
          "Safari Zone (Area 2)",
          "Safari Zone (Area 3)",
          "Safari Zone (Entrance)",
          "Safari Zone (Rest house 1)",
          "Safari Zone (Prize house)",
          "Safari Zone (Rest house 2)",
          "Safari Zone (Rest house 3)",
          "Safari Zone (Rest house 4)",
          "Cerulean Cave",
          "Cerulean Cave 1F",
          "Cerulean Cave B1F",
          "Name Rater's house (Lavender Town)",
          "Cerulean City (Gym Badge man)",
          "INVALID",
          "Rock Tunnel",
          "Silph Co. 9F",
          "Silph Co. 10F",
          "Silph Co. 11F",
          "Silph Co. Lift",
          "INVALID",
          "INVALID",
          "Cable Club Trade Center(*)",
          "Cable Club Colosseum(*)",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "Lorelei's room",
          "Bruno's room",
          "Agatha's room",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "INVALID",
          "(Indoor-Outside Map Handler)"
    // (Remaining entries default to empty string if not explicitly provided.)
};

const int Gen1MapLookup::MapIDNo[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

const std::string Gen1MapLookup::MapIDHex[256] = {
    "0x00", "0x01", "0x02", "0x03", "0x04", "0x05", "0x06", "0x07", "0x08", "0x09", "0x0A", "0x0B", "0x0C", "0x0D", "0x0E", "0x0F",
    "0x10", "0x11", "0x12", "0x13", "0x14", "0x15", "0x16", "0x17", "0x18", "0x19", "0x1A", "0x1B", "0x1C", "0x1D", "0x1E", "0x1F",
    "0x20", "0x21", "0x22", "0x23", "0x24", "0x25", "0x26", "0x27", "0x28", "0x29", "0x2A", "0x2B", "0x2C", "0x2D", "0x2E", "0x2F",
    "0x30", "0x31", "0x32", "0x33", "0x34", "0x35", "0x36", "0x37", "0x38", "0x39", "0x3A", "0x3B", "0x3C", "0x3D", "0x3E", "0x3F",
    "0x40", "0x41", "0x42", "0x43", "0x44", "0x45", "0x46", "0x47", "0x48", "0x49", "0x4A", "0x4B", "0x4C", "0x4D", "0x4E", "0x4F",
    "0x50", "0x51", "0x52", "0x53", "0x54", "0x55", "0x56", "0x57", "0x58", "0x59", "0x5A", "0x5B", "0x5C", "0x5D", "0x5E", "0x5F",
    "0x60", "0x61", "0x62", "0x63", "0x64", "0x65", "0x66", "0x67", "0x68", "0x69", "0x6A", "0x6B", "0x6C", "0x6D", "0x6E", "0x6F",
    "0x70", "0x71", "0x72", "0x73", "0x74", "0x75", "0x76", "0x77", "0x78", "0x79", "0x7A", "0x7B", "0x7C", "0x7D", "0x7E", "0x7F",
    "0x80", "0x81", "0x82", "0x83", "0x84", "0x85", "0x86", "0x87", "0x88", "0x89", "0x8A", "0x8B", "0x8C", "0x8D", "0x8E", "0x8F",
    "0x90", "0x91", "0x92", "0x93", "0x94", "0x95", "0x96", "0x97", "0x98", "0x99", "0x9A", "0x9B", "0x9C", "0x9D", "0x9E", "0x9F",
    "0xA0", "0xA1", "0xA2", "0xA3", "0xA4", "0xA5", "0xA6", "0xA7", "0xA8", "0xA9", "0xAA", "0xAB", "0xAC", "0xAD", "0xAE", "0xAF",
    "0xB0", "0xB1", "0xB2", "0xB3", "0xB4", "0xB5", "0xB6", "0xB7", "0xB8", "0xB9", "0xBA", "0xBB", "0xBC", "0xBD", "0xBE", "0xBF",
    "0xC0", "0xC1", "0xC2", "0xC3", "0xC4", "0xC5", "0xC6", "0xC7", "0xC8", "0xC9", "0xCA", "0xCB", "0xCC", "0xCD", "0xCE", "0xCF",
    "0xD0", "0xD1", "0xD2", "0xD3", "0xD4", "0xD5", "0xD6", "0xD7", "0xD8", "0xD9", "0xDA", "0xDB", "0xDC", "0xDD", "0xDE", "0xDF",
    "0xE0", "0xE1", "0xE2", "0xE3", "0xE4", "0xE5", "0xE6", "0xE7", "0xE8", "0xE9", "0xEA", "0xEB", "0xEC", "0xED", "0xEE", "0xEF",
    "0xF0", "0xF1", "0xF2", "0xF3", "0xF4", "0xF5", "0xF6", "0xF7", "0xF8", "0xF9", "0xFA", "0xFB", "0xFC", "0xFD", "0xFE", "0xFF",
};

std::string Gen1MapLookup::NameFromId(u8 mapId) {
    const std::size_t idx = static_cast<std::size_t>(mapId);
    // idx is always 0..255 for u8, but keep the check for safety.
    if (idx >= 256) return "INVALID";

    const std::string& n = MapIDName[idx];
    return n.empty() ? std::string("INVALID") : n;
}

// =========================================================
// Gen1SpeciesLookup
// =========================================================
const std::string Gen1SpeciesLookup::SpeciesName[256] = {
    /*0x00*/ "INVALID",
    /*0x01*/ "RHYDON",
    /*0x02*/ "KANGASKHAN",
    /*0x03*/ "NIDORAN_M",
    /*0x04*/ "CLEFAIRY",
    /*0x05*/ "SPEAROW",
    /*0x06*/ "VOLTORB",
    /*0x07*/ "NIDOKING",
    /*0x08*/ "SLOWBRO",
    /*0x09*/ "IVYSAUR",
    /*0x0A*/ "EXEGGUTOR",
    /*0x0B*/ "LICKITUNG",
    /*0x0C*/ "EXEGGCUTE",
    /*0x0D*/ "GRIMER",
    /*0x0E*/ "GENGAR",
    /*0x0F*/ "NIDORAN_F",
    /*0x10*/ "NIDOQUEEN",
    /*0x11*/ "CUBONE",
    /*0x12*/ "RHYHORN",
    /*0x13*/ "LAPRAS",
    /*0x14*/ "ARCANINE",
    /*0x15*/ "MEW",
    /*0x16*/ "GYARADOS",
    /*0x17*/ "SHELLDER",
    /*0x18*/ "TENTACOOL",
    /*0x19*/ "GASTLY",
    /*0x1A*/ "SCYTHER",
    /*0x1B*/ "STARYU",
    /*0x1C*/ "BLASTOISE",
    /*0x1D*/ "PINSIR",
    /*0x1E*/ "TANGELA",
    /*0x1F*/ "MISSINGNO",
    /*0x20*/ "MISSINGNO",
    /*0x21*/ "GROWLITHE",
    /*0x22*/ "ONIX",
    /*0x23*/ "FEAROW",
    /*0x24*/ "PIDGEY",
    /*0x25*/ "SLOWPOKE",
    /*0x26*/ "KADABRA",
    /*0x27*/ "GRAVELER",
    /*0x28*/ "CHANSEY",
    /*0x29*/ "MACHOKE",
    /*0x2A*/ "MR_MIME",
    /*0x2B*/ "HITMONLEE",
    /*0x2C*/ "HITMONCHAN",
    /*0x2D*/ "ARBOK",
    /*0x2E*/ "PARASECT",
    /*0x2F*/ "PSYDUCK",
    /*0x30*/ "DROWZEE",
    /*0x31*/ "GOLEM",
    /*0x32*/ "MISSINGNO",
    /*0x33*/ "MAGMAR",
    /*0x34*/ "MISSINGNO",
    /*0x35*/ "ELECTABUZZ",
    /*0x36*/ "MAGNETON",
    /*0x37*/ "KOFFING",
    /*0x38*/ "MISSINGNO",
    /*0x39*/ "MANKEY",
    /*0x3A*/ "SEEL",
    /*0x3B*/ "DIGLETT",
    /*0x3C*/ "TAUROS",
    /*0x3D*/ "MISSINGNO",
    /*0x3E*/ "MISSINGNO",
    /*0x3F*/ "MISSINGNO",
    /*0x40*/ "FARFETCHD",
    /*0x41*/ "VENONAT",
    /*0x42*/ "DRAGONITE",
    /*0x43*/ "MISSINGNO",
    /*0x44*/ "MISSINGNO",
    /*0x45*/ "MISSINGNO",
    /*0x46*/ "DODUO",
    /*0x47*/ "POLIWAG",
    /*0x48*/ "JYNX",
    /*0x49*/ "MOLTRES",
    /*0x4A*/ "ARTICUNO",
    /*0x4B*/ "ZAPDOS",
    /*0x4C*/ "DITTO",
    /*0x4D*/ "MEOWTH",
    /*0x4E*/ "KRABBY",
    /*0x4F*/ "MISSINGNO",
    /*0x50*/ "MISSINGNO",
    /*0x51*/ "MISSINGNO",
    /*0x52*/ "VULPIX",
    /*0x53*/ "NINETALES",
    /*0x54*/ "PIKACHU",
    /*0x55*/ "RAICHU",
    /*0x56*/ "MISSINGNO",
    /*0x57*/ "MISSINGNO",
    /*0x58*/ "DRATINI",
    /*0x59*/ "DRAGONAIR",
    /*0x5A*/ "KABUTO",
    /*0x5B*/ "KABUTOPS",
    /*0x5C*/ "HORSEA",
    /*0x5D*/ "SEADRA",
    /*0x5E*/ "MISSINGNO",
    /*0x5F*/ "MISSINGNO",
    /*0x60*/ "SANDSHREW",
    /*0x61*/ "SANDSLASH",
    /*0x62*/ "OMANYTE",
    /*0x63*/ "OMASTAR",
    /*0x64*/ "JIGGLYPUFF",
    /*0x65*/ "WIGGLYTUFF",
    /*0x66*/ "EEVEE",
    /*0x67*/ "FLAREON",
    /*0x68*/ "JOLTEON",
    /*0x69*/ "VAPOREON",
    /*0x6A*/ "MACHOP",
    /*0x6B*/ "ZUBAT",
    /*0x6C*/ "EKANS",
    /*0x6D*/ "PARAS",
    /*0x6E*/ "POLIWHIRL",
    /*0x6F*/ "POLIWRATH",
    /*0x70*/ "WEEDLE",
    /*0x71*/ "KAKUNA",
    /*0x72*/ "BEEDRILL",
    /*0x73*/ "MISSINGNO",
    /*0x74*/ "DODRIO",
    /*0x75*/ "PRIMEAPE",
    /*0x76*/ "DUGTRIO",
    /*0x77*/ "VENOMOTH",
    /*0x78*/ "DEWGONG",
    /*0x79*/ "MISSINGNO",
    /*0x7A*/ "MISSINGNO",
    /*0x7B*/ "CATERPIE",
    /*0x7C*/ "METAPOD",
    /*0x7D*/ "BUTTERFREE",
    /*0x7E*/ "MACHAMP",
    /*0x7F*/ "MISSINGNO",
    /*0x80*/ "GOLDUCK",
    /*0x81*/ "HYPNO",
    /*0x82*/ "GOLBAT",
    /*0x83*/ "MEWTWO",
    /*0x84*/ "SNORLAX",
    /*0x85*/ "MAGIKARP",
    /*0x86*/ "MISSINGNO",
    /*0x87*/ "MISSINGNO",
    /*0x88*/ "MUK",
    /*0x89*/ "MISSINGNO",
    /*0x8A*/ "KINGLER",
    /*0x8B*/ "CLOYSTER",
    /*0x8C*/ "MISSINGNO",
    /*0x8D*/ "ELECTRODE",
    /*0x8E*/ "CLEFABLE",
    /*0x8F*/ "WEEZING",
    /*0x90*/ "PERSIAN",
    /*0x91*/ "MAROWAK",
    /*0x92*/ "MISSINGNO",
    /*0x93*/ "HAUNTER",
    /*0x94*/ "ABRA",
    /*0x95*/ "ALAKAZAM",
    /*0x96*/ "PIDGEOTTO",
    /*0x97*/ "PIDGEOT",
    /*0x98*/ "STARMIE",
    /*0x99*/ "BULBASAUR",
    /*0x9A*/ "VENUSAUR",
    /*0x9B*/ "TENTACRUEL",
    /*0x9C*/ "MISSINGNO",
    /*0x9D*/ "GOLDEEN",
    /*0x9E*/ "SEAKING",
    /*0x9F*/ "MISSINGNO",
    /*0xA0*/ "MISSINGNO",
    /*0xA1*/ "MISSINGNO",
    /*0xA2*/ "MISSINGNO",
    /*0xA3*/ "PONYTA",
    /*0xA4*/ "RAPIDASH",
    /*0xA5*/ "RATTATA",
    /*0xA6*/ "RATICATE",
    /*0xA7*/ "NIDORINO",
    /*0xA8*/ "NIDORINA",
    /*0xA9*/ "GEODUDE",
    /*0xAA*/ "PORYGON",
    /*0xAB*/ "AERODACTYL",
    /*0xAC*/ "MISSINGNO",
    /*0xAD*/ "MAGNEMITE",
    /*0xAE*/ "MISSINGNO",
    /*0xAF*/ "MISSINGNO",
    /*0xB0*/ "CHARMANDER",
    /*0xB1*/ "SQUIRTLE",
    /*0xB2*/ "CHARMELEON",
    /*0xB3*/ "WARTORTLE",
    /*0xB4*/ "CHARIZARD",
    /*0xB5*/ "MISSINGNO",
    /*0xB6*/ "MISSINGNO",
    /*0xB7*/ "MISSINGNO",
    /*0xB8*/ "MISSINGNO",
    /*0xB9*/ "ODDISH",
    /*0xBA*/ "GLOOM",
    /*0xBB*/ "VILEPLUME",
    /*0xBC*/ "BELLSPROUT",
    /*0xBD*/ "WEEPINBELL",
    /*0xBE*/ "VICTREEBEL",

    // 0xBF..0xFF are glitch/garbage indices in Gen I; keep as INVALID.
};

const int Gen1SpeciesLookup::SpeciesNo[256] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
    80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
    96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
    144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
    160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
    192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
    208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
    224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};
const int Gen1SpeciesLookup::PokeDex[256] = {
    /*  0 */ -1,
    /*  1 */ 153, /*  2 */  9, /*  3 */ 154, /*  4 */ 176, /*  5 */ 178, /*  6 */ 180, /*  7 */ 177, /*  8 */ 179,
    /*  9 */  28, /* 10 */ 123, /* 11 */ 124, /* 12 */ 125, /* 13 */ 112, /* 14 */ 113, /* 15 */ 114, /* 16 */  36,
    /* 17 */ 150, /* 18 */ 151, /* 19 */ 165, /* 20 */ 166, /* 21 */   5, /* 22 */  35, /* 23 */ 108, /* 24 */  45,
    /* 25 */  84, /* 26 */  85, /* 27 */  96, /* 28 */  97, /* 29 */  15, /* 30 */ 168, /* 31 */  16, /* 32 */   3,
    /* 33 */ 167, /* 34 */   7, /* 35 */   4, /* 36 */ 142, /* 37 */  82, /* 38 */  83, /* 39 */ 100, /* 40 */ 101,
    /* 41 */ 107, /* 42 */ 130, /* 43 */ 185, /* 44 */ 186, /* 45 */ 187, /* 46 */ 109, /* 47 */  46, /* 48 */  65,
    /* 49 */ 119, /* 50 */  59, /* 51 */ 118, /* 52 */  77, /* 53 */ 144, /* 54 */  47, /* 55 */ 128, /* 56 */  57,
    /* 57 */ 117, /* 58 */  33, /* 59 */  20, /* 60 */  71, /* 61 */ 110, /* 62 */ 111, /* 63 */ 148, /* 64 */  38,
    /* 65 */ 149, /* 66 */ 106, /* 67 */  41, /* 68 */ 126, /* 69 */ 188, /* 70 */ 189, /* 71 */ 190, /* 72 */  24,
    /* 73 */ 155, /* 74 */ 169, /* 75 */  39, /* 76 */  49, /* 77 */ 163, /* 78 */ 164, /* 79 */  37, /* 80 */   8,
    /* 81 */ 173, /* 82 */  54, /* 83 */  64, /* 84 */  70, /* 85 */ 116, /* 86 */  58, /* 87 */ 120, /* 88 */  13,
    /* 89 */ 136, /* 90 */  23, /* 91 */ 139, /* 92 */  25, /* 93 */ 147, /* 94 */  14, /* 95 */  34, /* 96 */  48,
    /* 97 */ 129, /* 98 */  78, /* 99 */ 138, /*100 */   6, /*101 */ 141, /*102 */  12, /*103 */  10, /*104 */  17,
    /*105 */ 145, /*106 */  43, /*107 */  44, /*108 */  11, /*109 */  55, /*110 */ 143, /*111 */  18, /*112 */   1,
    /*113 */  40, /*114 */  30, /*115 */   2, /*116 */  92, /*117 */  93, /*118 */ 157, /*119 */ 158, /*120 */  27,
    /*121 */ 152, /*122 */  42, /*123 */  26, /*124 */  72, /*125 */  53, /*126 */  51, /*127 */  29, /*128 */  60,
    /*129 */ 133, /*130 */  22, /*131 */  19, /*132 */  76, /*133 */ 102, /*134 */ 105, /*135 */ 104, /*136 */ 103,
    /*137 */ 170, /*138 */  98, /*139 */  99, /*140 */  90, /*141 */  91, /*142 */ 171, /*143 */ 132, /*144 */  74,
    /*145 */  75, /*146 */  73, /*147 */  88, /*148 */  89, /*149 */  66, /*150 */ 131, /*151 */  21,

    /*152 */ -1, /*153 */ -1, /*154 */ -1, /*155 */ -1, /*156 */ -1, /*157 */ -1, /*158 */ -1, /*159 */ -1,
    /*160 */ -1, /*161 */ -1, /*162 */ -1, /*163 */ -1, /*164 */ -1, /*165 */ -1, /*166 */ -1, /*167 */ -1,
    /*168 */ -1, /*169 */ -1, /*170 */ -1, /*171 */ -1, /*172 */ -1, /*173 */ -1, /*174 */ -1, /*175 */ -1,
    /*176 */ -1, /*177 */ -1, /*178 */ -1, /*179 */ -1, /*180 */ -1, /*181 */ -1, /*182 */ -1, /*183 */ -1,
    /*184 */ -1, /*185 */ -1, /*186 */ -1, /*187 */ -1, /*188 */ -1, /*189 */ -1, /*190 */ -1, /*191 */ -1,
    /*192 */ -1, /*193 */ -1, /*194 */ -1, /*195 */ -1, /*196 */ -1, /*197 */ -1, /*198 */ -1, /*199 */ -1,
    /*200 */ -1, /*201 */ -1, /*202 */ -1, /*203 */ -1, /*204 */ -1, /*205 */ -1, /*206 */ -1, /*207 */ -1,
    /*208 */ -1, /*209 */ -1, /*210 */ -1, /*211 */ -1, /*212 */ -1, /*213 */ -1, /*214 */ -1, /*215 */ -1,
    /*216 */ -1, /*217 */ -1, /*218 */ -1, /*219 */ -1, /*220 */ -1, /*221 */ -1, /*222 */ -1, /*223 */ -1,
    /*224 */ -1, /*225 */ -1, /*226 */ -1, /*227 */ -1, /*228 */ -1, /*229 */ -1, /*230 */ -1, /*231 */ -1,
    /*232 */ -1, /*233 */ -1, /*234 */ -1, /*235 */ -1, /*236 */ -1, /*237 */ -1, /*238 */ -1, /*239 */ -1,
    /*240 */ -1, /*241 */ -1, /*242 */ -1, /*243 */ -1, /*244 */ -1, /*245 */ -1, /*246 */ -1, /*247 */ -1,
    /*248 */ -1, /*249 */ -1, /*250 */ -1, /*251 */ -1, /*252 */ -1, /*253 */ -1, /*254 */ -1, /*255 */ -1
};

const std::string Gen1SpeciesLookup::SpeciesHex[256]= {
        "0x00","0x01","0x02","0x03","0x04","0x05","0x06","0x07","0x08","0x09","0x0a","0x0b","0x0c","0x0d","0x0e","0x0f",
        "0x10","0x11","0x12","0x13","0x14","0x15","0x16","0x17","0x18","0x19","0x1a","0x1b","0x1c","0x1d","0x1e","0x1f",
        "0x20","0x21","0x22","0x23","0x24","0x25","0x26","0x27","0x28","0x29","0x2a","0x2b","0x2c","0x2d","0x2e","0x2f",
        "0x30","0x31","0x32","0x33","0x34","0x35","0x36","0x37","0x38","0x39","0x3a","0x3b","0x3c","0x3d","0x3e","0x3f",
        "0x40","0x41","0x42","0x43","0x44","0x45","0x46","0x47","0x48","0x49","0x4a","0x4b","0x4c","0x4d","0x4e","0x4f",
        "0x50","0x51","0x52","0x53","0x54","0x55","0x56","0x57","0x58","0x59","0x5a","0x5b","0x5c","0x5d","0x5e","0x5f",
        "0x60","0x61","0x62","0x63","0x64","0x65","0x66","0x67","0x68","0x69","0x6a","0x6b","0x6c","0x6d","0x6e","0x6f",
        "0x70","0x71","0x72","0x73","0x74","0x75","0x76","0x77","0x78","0x79","0x7a","0x7b","0x7c","0x7d","0x7e","0x7f",
        "0x80","0x81","0x82","0x83","0x84","0x85","0x86","0x87","0x88","0x89","0x8a","0x8b","0x8c","0x8d","0x8e","0x8f",
        "0x90","0x91","0x92","0x93","0x94","0x95","0x96","0x97","0x98","0x99","0x9a","0x9b","0x9c","0x9d","0x9e","0x9f",
        "0xa0","0xa1","0xa2","0xa3","0xa4","0xa5","0xa6","0xa7","0xa8","0xa9","0xaa","0xab","0xac","0xad","0xae","0xaf",
        "0xb0","0xb1","0xb2","0xb3","0xb4","0xb5","0xb6","0xb7","0xb8","0xb9","0xba","0xbb","0xbc","0xbd","0xbe","0xbf",
        "0xc0","0xc1","0xc2","0xc3","0xc4","0xc5","0xc6","0xc7","0xc8","0xc9","0xca","0xcb","0xcc","0xcd","0xce","0xcf",
        "0xd0","0xd1","0xd2","0xd3","0xd4","0xd5","0xd6","0xd7","0xd8","0xd9","0xda","0xdb","0xdc","0xdd","0xde","0xdf",
        "0xe0","0xe1","0xe2","0xe3","0xe4","0xe5","0xe6","0xe7","0xe8","0xe9","0xea","0xeb","0xec","0xed","0xee","0xef",
        "0xf0","0xf1","0xf2","0xf3","0xf4","0xf5","0xf6","0xf7","0xf8","0xf9","0xfa","0xfb","0xfc","0xfd","0xfe","0xff"
    };

std::string Gen1SpeciesLookup::NameFromId(u8 speciesId) {
    const std::size_t idx = static_cast<std::size_t>(speciesId);
    if (idx >= 256) return "INVALID";
    const std::string& n = SpeciesName[idx];
    if (n.empty()) return "INVALID";
    return n;
}



// =========================================================
// Gen1TextCodec
// =========================================================

char Gen1TextCodec::ByteToAscii(u8 byte) {
    // Minimal charset for MVP (names): A-Z, 0-9, space, terminator.
    if (byte >= 0x80 && byte <= 0x99) return static_cast<char>('A' + (byte - 0x80));
    if (byte >= 0xA0 && byte <= 0xA9) return static_cast<char>('0' + (byte - 0xA0));
    if (byte == 0x7F) return ' ';
    if (byte == 0x50) return '\0';
    return '?';
}

u8 Gen1TextCodec::AsciiToByte(char c) {
    // Normalize lowercase to uppercase.
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');

    if (c >= 'A' && c <= 'Z') return static_cast<u8>(0x80 + (c - 'A'));
    if (c >= '0' && c <= '9') return static_cast<u8>(0xA0 + (c - '0'));
    if (c == ' ') return 0x7F;

    // Fallback to space for unsupported characters in MVP.
    return 0x7F;
}

std::string Gen1TextCodec::DecodeName(const SaveBuffer& sb, std::size_t off, std::size_t len) {
    const auto bytes = sb.Slice(off, len);

    std::string out;
    out.reserve(len);

    for (u8 b : bytes) {
        const char c = ByteToAscii(b);
        if (c == '\0') break;
        out.push_back(c);
    }

    return out;
}

void Gen1TextCodec::EncodeName(SaveBuffer& sb, std::size_t off, std::size_t len, std::string_view name) {
    if (len == 0) return;

    // Fill with terminators (0x50).
    std::vector<u8> out(len, 0x50);

    const std::size_t n = std::min(name.size(), len - 1);
    for (std::size_t i = 0; i < n; ++i) {
        out[i] = AsciiToByte(name[i]);
    }

    out[n] = 0x50;

    // Write into the buffer.
    sb.RequireRange(off, len);
    auto& bytes = sb.BytesMutable();
    std::copy(out.begin(), out.end(), bytes.begin() + static_cast<std::ptrdiff_t>(off));
}

// =========================================================
// BcdCodec
// =========================================================

static u32 bcdDigit(u8 nibble) {
    return (nibble <= 9) ? static_cast<u32>(nibble) : 0u;
}

u32 BcdCodec::ReadBcd3(const SaveBuffer& sb, std::size_t off) {
    const u8 b0 = sb.ReadU8(off);
    const u8 b1 = sb.ReadU8(off + 1);
    const u8 b2 = sb.ReadU8(off + 2);

    const u32 d0 = bcdDigit((b0 >> 4) & 0xF);
    const u32 d1 = bcdDigit(b0 & 0xF);
    const u32 d2 = bcdDigit((b1 >> 4) & 0xF);
    const u32 d3 = bcdDigit(b1 & 0xF);
    const u32 d4 = bcdDigit((b2 >> 4) & 0xF);
    const u32 d5 = bcdDigit(b2 & 0xF);

    return d0 * 100000u + d1 * 10000u + d2 * 1000u + d3 * 100u + d4 * 10u + d5;
}

void BcdCodec::WriteBcd3(SaveBuffer& sb, std::size_t off, u32 value) {
    if (value > 999999u) {
        throw std::out_of_range("WriteBcd3: value must be 0..999999");
    }

    const u32 d0 = (value / 100000u) % 10u;
    const u32 d1 = (value / 10000u) % 10u;
    const u32 d2 = (value / 1000u) % 10u;
    const u32 d3 = (value / 100u) % 10u;
    const u32 d4 = (value / 10u) % 10u;
    const u32 d5 = value % 10u;

    const u8 b0 = static_cast<u8>((d0 << 4) | d1);
    const u8 b1 = static_cast<u8>((d2 << 4) | d3);
    const u8 b2 = static_cast<u8>((d4 << 4) | d5);

    sb.WriteU8(off, b0);
    sb.WriteU8(off + 1, b1);
    sb.WriteU8(off + 2, b2);
}

u16 BcdCodec::ReadBcd2(const SaveBuffer& sb, std::size_t off) {
    const u8 b0 = sb.ReadU8(off);
    const u8 b1 = sb.ReadU8(off + 1);

    const u32 d0 = bcdDigit((b0 >> 4) & 0xF);
    const u32 d1 = bcdDigit(b0 & 0xF);
    const u32 d2 = bcdDigit((b1 >> 4) & 0xF);
    const u32 d3 = bcdDigit(b1 & 0xF);

    const u32 val = d0 * 1000u + d1 * 100u + d2 * 10u + d3;
    return static_cast<u16>(val);
}

void BcdCodec::WriteBcd2(SaveBuffer& sb, std::size_t off, u16 value) {
    if (value > 9999u) {
        throw std::out_of_range("WriteBcd2: value must be 0..9999");
    }

    const u32 v = static_cast<u32>(value);
    const u32 d0 = (v / 1000u) % 10u;
    const u32 d1 = (v / 100u) % 10u;
    const u32 d2 = (v / 10u) % 10u;
    const u32 d3 = v % 10u;

    const u8 b0 = static_cast<u8>((d0 << 4) | d1);
    const u8 b1 = static_cast<u8>((d2 << 4) | d3);

    sb.WriteU8(off, b0);
    sb.WriteU8(off + 1, b1);
}

// =========================================================
// Gen1Checksum
// =========================================================

static u8 sumAndInvert8(const SaveBuffer& sb, std::size_t startInclusive, std::size_t endInclusive) {
    if (endInclusive < startInclusive) {
        throw std::invalid_argument("Checksum: end < start");
    }

    u32 sum = 0;
    for (std::size_t i = startInclusive; i <= endInclusive; ++i) {
        sum += sb.ReadU8(i);
    }

    const u8 low = static_cast<u8>(sum & 0xFF);
    return static_cast<u8>(~low);
}

u8 Gen1Checksum::ComputeMain(const SaveBuffer& sb) {
    // Bulbapedia: sum bytes 0x2598..0x3522 inclusive; stored at 0x3523.
    return sumAndInvert8(sb, Gen1Layout::MainChecksumStart, Gen1Layout::MainChecksumEnd);
}

bool Gen1Checksum::ValidateMain(const SaveBuffer& sb) {
    const u8 expected = ComputeMain(sb);
    const u8 stored = sb.ReadU8(Gen1Layout::MainChecksumOff);
    return expected == stored;
}

void Gen1Checksum::FixMain(SaveBuffer& sb) {
    sb.WriteU8(Gen1Layout::MainChecksumOff, ComputeMain(sb));
}

u8 Gen1Checksum::ComputeBankAll(const SaveBuffer& sb, int bankIndex2or3) {
    if (bankIndex2or3 != 2 && bankIndex2or3 != 3) {
        throw std::invalid_argument("ComputeBankAll: bank index must be 2 or 3");
    }

    const std::size_t start = (bankIndex2or3 == 2) ? Gen1Layout::Bank2Base : Gen1Layout::Bank3Base;
    const std::size_t checksumOff = (bankIndex2or3 == 2) ? Gen1Layout::Bank2AllChecksumOff : Gen1Layout::Bank3AllChecksumOff;

    // Boxes occupy start .. checksumOff-1
    return sumAndInvert8(sb, start, checksumOff - 1);
}

bool Gen1Checksum::ValidateBankAll(const SaveBuffer& sb, int bankIndex2or3) {
    const std::size_t checksumOff = (bankIndex2or3 == 2) ? Gen1Layout::Bank2AllChecksumOff : Gen1Layout::Bank3AllChecksumOff;
    const u8 stored = sb.ReadU8(checksumOff);
    const u8 expected = ComputeBankAll(sb, bankIndex2or3);
    return stored == expected;
}

void Gen1Checksum::FixBankAll(SaveBuffer& sb, int bankIndex2or3) {
    const std::size_t checksumOff = (bankIndex2or3 == 2) ? Gen1Layout::Bank2AllChecksumOff : Gen1Layout::Bank3AllChecksumOff;
    sb.WriteU8(checksumOff, ComputeBankAll(sb, bankIndex2or3));
}

u8 Gen1Checksum::ComputeBox(const SaveBuffer& sb, int boxIndex1to12) {
    const std::size_t start = Gen1Layout::BoxBaseOffsetByIndex1to12(boxIndex1to12);
    const std::size_t end = start + Gen1Layout::BoxBlockSize - 1;
    return sumAndInvert8(sb, start, end);
}

bool Gen1Checksum::ValidateBox(const SaveBuffer& sb, int boxIndex1to12) {
    const std::size_t tableBase = Gen1Layout::BankPerBoxChecksumsBaseOffsetForBoxIndex1to12(boxIndex1to12);
    const int withinBank = (boxIndex1to12 <= 6) ? (boxIndex1to12 - 1) : (boxIndex1to12 - 7);

    const u8 stored = sb.ReadU8(tableBase + static_cast<std::size_t>(withinBank));
    const u8 expected = ComputeBox(sb, boxIndex1to12);
    return stored == expected;
}

void Gen1Checksum::FixBox(SaveBuffer& sb, int boxIndex1to12) {
    const std::size_t tableBase = Gen1Layout::BankPerBoxChecksumsBaseOffsetForBoxIndex1to12(boxIndex1to12);
    const int withinBank = (boxIndex1to12 <= 6) ? (boxIndex1to12 - 1) : (boxIndex1to12 - 7);

    sb.WriteU8(tableBase + static_cast<std::size_t>(withinBank), ComputeBox(sb, boxIndex1to12));
}

// =========================================================
// SaveValidator
// =========================================================

void SaveValidator::RequireExpectedSize(const SaveBuffer& sb) {
    if (sb.Size() != Gen1Layout::ExpectedSize) {
        std::ostringstream oss;
        oss << "Unexpected save size: 0x" << std::hex << sb.Size()
            << " (expected 0x" << Gen1Layout::ExpectedSize << ")";
        throw std::runtime_error(oss.str());
    }
}

bool SaveValidator::HasExpectedSize(const SaveBuffer& sb) {
    return sb.Size() == Gen1Layout::ExpectedSize;
}

bool SaveValidator::HasValidMainChecksum(const SaveBuffer& sb) {
    // If size is wrong, checksum check may throw; treat as invalid.
    try {
        return Gen1Checksum::ValidateMain(sb);
    } catch (...) {
        return false;
    }
}

} // namespace savegenie
