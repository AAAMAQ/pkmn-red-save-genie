//
//  SaveStructure.hpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Single-source-of-truth for the Generation I Pokémon save format (Pokémon Red/Blue/Yellow).
//   - Provides safe, bounds-checked byte access to a loaded .sav buffer.
//   - Defines the bank layout, key offsets, and low-level codecs (Gen I text, BCD, checksums).
//
//  Owns:
//   - SaveBuffer: encapsulates raw bytes and exposes safe Read/Write helpers.
//   - Gen1Layout: bank bases + offsets/lengths for core fields (expand over time).
//   - Gen1TextCodec: minimal Gen I text encoding/decoding (names first; expandable later).
//   - BcdCodec: money/coins helpers.
//   - Gen1Checksum: compute/validate/fix routines for main and box banks.
//
//  Does NOT:
//   - Perform file I/O (see FileManipulation).
//   - Provide formatted dumps (see ReadOnlyData).
//   - Apply user/business edits (see WriteOnlyData).
//

#ifndef SaveStructure_hpp
#define SaveStructure_hpp

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
namespace savegenie {

// =========================
// Basic types
// =========================
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

// =========================
// SaveBuffer (safe byte access)
// =========================
class SaveBuffer {
public:
    using Bytes = std::vector<u8>;

    SaveBuffer();
    explicit SaveBuffer(Bytes bytes);

    // Size of the underlying buffer.
    std::size_t Size() const;

    // Read-only access to raw bytes (for writing out to disk).
    const Bytes& BytesView() const;

    // Mutable access (used only by editing layers).
    Bytes& BytesMutable();

    // --- Bounds checking ---
    void RequireRange(std::size_t off, std::size_t len) const;

    // --- Basic reads ---
    u8  ReadU8(std::size_t off) const;
    u16 ReadU16LE(std::size_t off) const;
    u32 ReadU24BE(std::size_t off) const; // 3 bytes: [hi][mid][lo]

    // --- Basic writes ---
    void WriteU8(std::size_t off, u8 v);
    void WriteU16LE(std::size_t off, u16 v);
    void WriteU24BE(std::size_t off, u32 v);

    // --- Bit helpers ---
    bool GetBit(std::size_t byteOff, u8 bitIndex0to7) const;
    void SetBit(std::size_t byteOff, u8 bitIndex0to7, bool value);

    // --- Slices ---
    Bytes Slice(std::size_t off, std::size_t len) const;

private:
    Bytes bytes_;
};

// =========================
// Layout + offsets
// =========================
class Gen1Layout {
public:
    // A standard Gen I SRAM save is 32 KiB.
    // Sometimes the expected size may be larger instead of 0x8000 it would be 0x802c; would not be an issue but a warning would show for secure editing
    static constexpr std::size_t ExpectedSize = 0x8000;

    // Bank bases within the .sav file.
    static constexpr std::size_t Bank0Base = 0x0000;
    static constexpr std::size_t Bank1Base = 0x2000;
    static constexpr std::size_t Bank2Base = 0x4000;
    static constexpr std::size_t Bank3Base = 0x6000;

    // Bank size.
    static constexpr std::size_t BankSize  = 0x2000;
    
    // --- Bank 0 layout (0x0000..0x1FFF) ---
    // Bank 0 is mostly scratch/unused, but it contains the Hall of Fame records.
    // NOTE: Bank 0 is NOT checksum-protected, so readers should parse defensively.

    // Sprite scratch buffers (runtime buffers; generally not meaningful to edit)
    static constexpr std::size_t SpriteBuffer0Off = 0x0000;
    static constexpr std::size_t SpriteBufferLen  = 0x0188; // 0x188 bytes each

    static constexpr std::size_t SpriteBuffer1Off = 0x0188;
    static constexpr std::size_t SpriteBuffer2Off = 0x0310;

    // Unused block before Hall of Fame
    static constexpr std::size_t Bank0Unused0Off  = 0x0498;
    static constexpr std::size_t Bank0Unused0Len  = 0x0100;

    // Hall of Fame records block
    static constexpr std::size_t HallOfFameOff    = 0x0598;
    static constexpr std::size_t HallOfFameLen    = 0x12C0;

    // Remaining unused space after Hall of Fame
    static constexpr std::size_t Bank0Unused1Off  = 0x1858;
    static constexpr std::size_t Bank0Unused1Len  = 0x07A8;

    // --- Hall of Fame record format ---
    // Up to 50 records, each 0x60 bytes, each record contains 6 Pokémon entries.
    // Each Pokémon entry is 0x10 bytes:
    //   +0x00 speciesId (u8)
    //   +0x01 level (u8)
    //   +0x02..+0x0C name (0x0B bytes, Gen I text, 0x50 terminator)
    static constexpr int HallOfFameMaxRecords            = 50;
    static constexpr std::size_t HallOfFameRecordSize    = 0x0060;
    static constexpr int HallOfFameMonsPerRecord         = 6;
    static constexpr std::size_t HallOfFameMonEntrySize  = 0x0010;

    // Bank 1 field: Hall of Fame record count
    static constexpr std::size_t HallOfFameRecordCountOff = 0x284E; // 1 byte
    // --- Core Bank 1 offsets (MVP fields) ---
    static constexpr std::size_t TrainerNameOff   = 0x2598;
    static constexpr std::size_t TrainerNameLen   = 11;    // includes terminator

    static constexpr std::size_t PokedexOwnedOff  = 0x25A3;
    static constexpr std::size_t PokedexSeenOff   = 0x25B6;
    static constexpr std::size_t PokedexBitsLen   = 0x13;  // 19 bytes

    static constexpr std::size_t BagItemsOff      = 0x25C9;
    static constexpr std::size_t BagItemsLen      = 0x2A;  // 42 bytes

    static constexpr std::size_t MoneyOff         = 0x25F3; // 3 bytes BCD
    static constexpr std::size_t MoneyLen         = 3;

    static constexpr std::size_t RivalNameOff     = 0x25F6;
    static constexpr std::size_t RivalNameLen     = 11;

    static constexpr std::size_t OptionsOff       = 0x2601;
    static constexpr std::size_t BadgesOff        = 0x2602;
    static constexpr std::size_t LetterDelayOff   = 0x2604;

    static constexpr std::size_t TrainerIdOff     = 0x2605; // u16 not little-endian but big-endian

    static constexpr std::size_t MusicIdOff       = 0x2607;
    static constexpr std::size_t MusicBankOff     = 0x2608;
    static constexpr std::size_t ContrastOff      = 0x2609;

    static constexpr std::size_t MapIdOff         = 0x260A;
    // Note: some docs list X/Y swapped; we follow Bulbapedia Gen I save structure.
    static constexpr std::size_t YCoordOff        = 0x260D;
    static constexpr std::size_t XCoordOff        = 0x260E;

    // Playtime region (hours / maxed byte / minutes / seconds / frames)
    static constexpr std::size_t PlayTimeHoursOff  = 0x2CED;
    static constexpr std::size_t PlayTimeMaxedOff  = 0x2CEE;
    static constexpr std::size_t PlayTimeMinutesOff= 0x2CEF;
    static constexpr std::size_t PlayTimeSecondsOff= 0x2CF0;
    static constexpr std::size_t PlayTimeFramesOff = 0x2CF1;

    // Coins (slot machine) are stored as 2-byte BCD at 0x2850.
    static constexpr std::size_t CoinsOff          = 0x2850;
    static constexpr std::size_t CoinsLen          = 2;

    // --- Checksums (Main bank 1) ---
    // Checksum is stored at 0x3523 and computed over 0x2598..0x3522 inclusive.
    static constexpr std::size_t MainChecksumStart = 0x2598;
    static constexpr std::size_t MainChecksumEnd   = 0x3522; // inclusive
    static constexpr std::size_t MainChecksumOff   = 0x3523;

    // --- PC Boxes (Banks 2 and 3) ---
    // Each full box block is 0x462 bytes.
    static constexpr std::size_t BoxBlockSize      = 0x0462;

    // Bank 2 boxes (1-6)
    static constexpr std::size_t Box1Off           = 0x4000;
    static constexpr std::size_t Box2Off           = 0x4462;
    static constexpr std::size_t Box3Off           = 0x48C4;
    static constexpr std::size_t Box4Off           = 0x4D26;
    static constexpr std::size_t Box5Off           = 0x5188;
    static constexpr std::size_t Box6Off           = 0x55EA;
    static constexpr std::size_t Bank2AllChecksumOff   = 0x5A4C;
    static constexpr std::size_t Bank2BoxChecksumsOff  = 0x5A4D; // 6 bytes (one per box)

    // Bank 3 boxes (7-12)
    static constexpr std::size_t Box7Off           = 0x6000;
    static constexpr std::size_t Box8Off           = 0x6462;
    static constexpr std::size_t Box9Off           = 0x68C4;
    static constexpr std::size_t Box10Off          = 0x6D26;
    static constexpr std::size_t Box11Off          = 0x7188;
    static constexpr std::size_t Box12Off          = 0x75EA;
    static constexpr std::size_t Bank3AllChecksumOff   = 0x7A4C;
    static constexpr std::size_t Bank3BoxChecksumsOff  = 0x7A4D; // 6 bytes

    // Helpers for iterating boxes.
    static std::size_t BoxBaseOffsetByIndex1to12(int boxIndex1to12);
    static std::size_t BankAllChecksumOffsetForBoxIndex1to12(int boxIndex1to12);
    static std::size_t BankPerBoxChecksumsBaseOffsetForBoxIndex1to12(int boxIndex1to12);
};

// =========================
// Gen I Species Lookup
// =========================
// Source: Bulbapedia
// Note:
// - ID of each species uses the Gen I internal index (0x00..0xFF)
// - This lookup is standardized to the internal SpeciesID (not Pokédex number)
//
class Gen1SpeciesLookup {
public:
    static const std::string SpeciesName[256];
    static const int SpeciesNo[256];
    static const std::string SpeciesHex[256];

    // Pokédex number (index) -> Gen I internal SpeciesID mapping.
    // Example: PokeDex[1] (Bulbasaur) -> 0x99 (153)
    // Invalid/unused entries should be -1.
    static const int PokeDex[256];

    static std::string NameFromId(u8 speciesId);
};

// =========================
// Gen I Map ID lookup (0x00..0xFF)
// =========================
// Source: "List of maps by index number (Generation I)" (Glitch City Wiki PDF).
// Any entry marked Unused/Invalid in the source is normalized here to "INVALID".
//
// Notes:
//  - Map IDs are dense (0..255), so a fixed-size lookup is simplest.
//  - We store three parallel vectors (as requested):
//      MapIDName[256] -> human-readable map name (or "INVALID")
//      MapIDNo[256]   -> decimal ID (0..255)
//      MapIDHex[256]  -> hex string ("0x00".."0xFF")
//



class Gen1MapLookup {
public:
    static const std::string MapIDName[256]; // size 256
    static const int MapIDNo[256];           // size 256
    static const std::string MapIDHex[256];  // size 256
    static std::string NameFromId(u8 mapId);
};

// =========================
// Gen I text codec (minimal; names first)
// =========================
class Gen1TextCodec {
public:
    // Decode an in-save name field (Gen I charset) into ASCII.
    // Stops at 0x50 terminator or length.
    static std::string DecodeName(const SaveBuffer& sb, std::size_t off, std::size_t len);

    // Encode ASCII into Gen I charset and write into the save.
    // Writes a 0x50 terminator and pads remaining bytes with 0x50.
    static void EncodeName(SaveBuffer& sb, std::size_t off, std::size_t len, std::string_view name);

    // Expose per-character conversions (useful for debugging).
    static char ByteToAscii(u8 byte);
    static u8   AsciiToByte(char c);
};

// =========================
// BCD codec (Money / Coins)
// =========================
class BcdCodec {
public:
    // Read 3-byte BCD (money) into an integer.
    static u32 ReadBcd3(const SaveBuffer& sb, std::size_t off);

    // Write integer into 3-byte BCD.
    // Valid range for Gen I money is 0..999999.
    static void WriteBcd3(SaveBuffer& sb, std::size_t off, u32 value);

    // Read 2-byte BCD (coins) into an integer.
    static u16 ReadBcd2(const SaveBuffer& sb, std::size_t off);

    // Write integer into 2-byte BCD.
    // Coins are typically 0..9999.
    static void WriteBcd2(SaveBuffer& sb, std::size_t off, u16 value);
};

// =========================
// Checksums
// =========================
class Gen1Checksum {
public:
    // Main checksum for Bank 1.
    static u8 ComputeMain(const SaveBuffer& sb);
    static bool ValidateMain(const SaveBuffer& sb);
    static void FixMain(SaveBuffer& sb);

    // Bank checksum for Banks 2 and 3 (the "all checksum" byte).
    // Note: This is NOT the same as the per-box checksums.
    static u8 ComputeBankAll(const SaveBuffer& sb, int bankIndex2or3);
    static bool ValidateBankAll(const SaveBuffer& sb, int bankIndex2or3);
    static void FixBankAll(SaveBuffer& sb, int bankIndex2or3);

    // Per-box checksums (one byte per box).
    // These are required if you edit box data.
    static u8 ComputeBox(const SaveBuffer& sb, int boxIndex1to12);
    static bool ValidateBox(const SaveBuffer& sb, int boxIndex1to12);
    static void FixBox(SaveBuffer& sb, int boxIndex1to12);
};

// =========================
// Basic save validation
// =========================
class SaveValidator {
public:
    // Throws if size is unexpected.
    static void RequireExpectedSize(const SaveBuffer& sb);

    // Non-throwing checks for UX.
    static bool HasExpectedSize(const SaveBuffer& sb);
    static bool HasValidMainChecksum(const SaveBuffer& sb);
};

} // namespace savegenie

#endif /* SaveStructure_hpp */
 
