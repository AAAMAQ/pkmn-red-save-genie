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
