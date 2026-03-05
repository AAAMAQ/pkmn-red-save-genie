// =========================================================
// WriteOnlyData.hpp
// Pkmn Red Save Genie
//
// Safe editing layer for Gen I Pokémon Red/Blue saves.
//
// Design rules:
//  - No arbitrary offset writes.
//  - All edits go through validated setters.
//  - Always recompute required checksums after mutation.
//  - File naming (BACKUP)/(EDITED) is handled by main.cpp + FileManipulation.
//
// This header provides declarations only; implementations live in WriteOnlyData.cpp.
// =========================================================

#ifndef WriteOnlyData_hpp
#define WriteOnlyData_hpp

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

#include "SaveStructure.hpp"

namespace savegenie {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

// =========================================================
// Result / Error model
// =========================================================

enum class EditStatus {
    Ok = 0,

    // Generic
    InvalidArgument,
    OutOfRange,
    InvalidSave,

    // Validation
    InvalidName,
    InvalidItemId,
    InvalidQuantity,
    ListFull,
    ItemNotFound,

    // Integrity
    ChecksumMismatch,
};

class EditMessage {
public:
    EditStatus status = EditStatus::Ok;
    std::string message;

    EditMessage() = default;
    EditMessage(EditStatus s, std::string m) : status(s), message(std::move(m)) {}

    bool Ok() const { return status == EditStatus::Ok; }
};

class EditLog {
public:
    std::vector<std::string> lines;

    void Add(const std::string& line) { lines.push_back(line); }
    std::string ToString() const;
};

// =========================================================
// MVP Edit Request
// =========================================================

class EditRequest {
public:
    // Names (Gen I text encoding)
    std::optional<std::string> newTrainerName;
    std::optional<std::string> newRivalName;

    // Money (0..999999) stored as 3-byte BCD
    std::optional<u32> newMoney;

    // Coins (0..9999) stored as 2-byte BCD
    std::optional<u16> newCoins;

    // Badges bitfield (8 bits)
    std::optional<u8> newBadges;

    // Safe location edit
    std::optional<u8>  newMapId;
    std::optional<u8>  newX;
    std::optional<u8>  newY;
};

// =========================================================
// Item list editing
// =========================================================

enum class ItemListKind {
    Bag = 0,
    PCItemBox = 1,
};

class ItemStack {
public:
    u8 itemId = 0;
    u8 quantity = 0;

    std::string itemName; // optional convenience (filled by code)
    std::string itemHex;  // optional convenience (filled by code)
};

class ItemEditRequest {
public:
    ItemListKind list = ItemListKind::Bag;

    // Add / set
    u8 itemId = 0;
    u8 quantity = 0;

    // Safety policy (defaults)
    u8 minQuantity = 1;
    u8 maxQuantity = 99; // “human safe” cap (1 byte allows up to 255)

    bool allowInvalidIds = false; // if false, reject IDs whose lookup name is INVALID
};

// =========================================================
// Future Pokémon editing placeholders
// =========================================================

enum class PokemonSlotKind {
    Party = 0,
    CurrentBox = 1,
    PCBox = 2,
};

class PokemonEditRequest {
public:
    PokemonSlotKind kind = PokemonSlotKind::Party;

    // Which slot
    int boxIndex1to12 = 1; // for PCBox
    int slotIndex0to19 = 0;

    // Minimal safe fields (expand later)
    std::optional<u8> newSpeciesId;
    std::optional<u8> newLevel;
    std::optional<std::string> newNickname;
    std::optional<std::string> newOtName;
};

// =========================================================
// WriteOnlyData (safe mutation engine)
// =========================================================

class WriteOnlyData {
public:
    // The editor operates directly on a SaveBuffer (mutable byte buffer).
    explicit WriteOnlyData(SaveBuffer& buffer);

    // ---------- MVP setters ----------
    EditMessage SetTrainerName(const std::string& name);
    EditMessage SetRivalName(const std::string& name);
    EditMessage SetMoney(u32 money);
    EditMessage SetCoins(u16 coins);
    EditMessage SetBadges(u8 badgesBitfield);
    EditMessage SetLocation(u8 mapId, u8 x, u8 y);

    // Apply an EditRequest as one transaction (recommended).
    // Implementations should:
    //  - validate inputs
    //  - apply mutations
    //  - write checksum(s)
    //  - append log lines
    EditMessage Apply(const EditRequest& req, EditLog* log = nullptr);

    // ---------- Item edits ----------
    // Read current list
    std::vector<ItemStack> ReadItemList(ItemListKind kind, bool includeNamesAndHex = true) const;

    // Add new or increase quantity if already present (policy-defined)
    EditMessage AddOrUpdateItem(const ItemEditRequest& req, EditLog* log = nullptr);

    // Remove item entirely
    EditMessage RemoveItem(ItemListKind kind, u8 itemId, EditLog* log = nullptr);

    // Set exact quantity for an item; if qty==0 may remove (implementation-defined)
    EditMessage SetItemQuantity(ItemListKind kind, u8 itemId, u8 quantity, EditLog* log = nullptr);

    // ---------- Future Pokémon edits (placeholders) ----------
    EditMessage ApplyPokemonEdit(const PokemonEditRequest& req, EditLog* log = nullptr);

    // ---------- Integrity ----------
    // Recompute and write required checksum(s) for a mutated save.
    EditMessage FixChecksums(EditLog* log = nullptr);

    // Validate current save integrity (size + known checksums)
    EditMessage Validate() const;

    // A plain-English edit report similar to DumpFullSummary.
    // (This describes WHAT changed, not the whole save.)
    std::string DumpFullEditLog(const EditLog& log) const;

    // A full, user-facing edit output (mirrors the style of DumpFullSummary).
    // Intended to be printed after Apply(...).
    std::string DumpFullEdit(const EditLog& log) const;

private:
    SaveBuffer& buffer_;

    // ----- Internal helpers -----
    static bool IsValidGen1NameChar(char c);
    static EditMessage ValidateGen1Name(const std::string& name, std::size_t maxLen);

    static EditMessage ValidateMoney(u32 money);
    static EditMessage ValidateCoins(u16 coins);

    static EditMessage ValidateItemId(u8 itemId, bool allowInvalidIds);
    static EditMessage ValidateQuantity(u8 qty, u8 minQty, u8 maxQty);

    // Encode helpers (use SaveStructure codecs)
    EditMessage WriteTrainerNameBytes(const std::string& name);
    EditMessage WriteRivalNameBytes(const std::string& name);

    // List manipulation (Bag / PC Item Box)
    struct ListRegion {
        std::size_t base = 0;
        std::size_t len = 0;
        std::size_t countOff = 0;
        std::size_t pairsOff = 0;
        int maxPairs = 0;
    };

    static ListRegion RegionFor(ItemListKind kind);

    // Low-level list operations (do not expose publicly)
    std::vector<ItemStack> ReadItemListInternal(const ListRegion& r, bool includeNamesAndHex) const;
    EditMessage WriteItemListInternal(const ListRegion& r, const std::vector<ItemStack>& items, EditLog* log);
};

} // namespace savegenie

#endif /* WriteOnlyData_hpp */
