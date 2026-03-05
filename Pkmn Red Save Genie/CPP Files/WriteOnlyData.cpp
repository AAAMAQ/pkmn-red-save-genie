//
//  WriteOnlyData.cpp
//  Pkmn Red Save Genie
//
//  Created by MAQ on 13/02/26.
//

// =========================================================
// WriteOnlyData.cpp
// Pkmn Red Save Genie
//
// Safe editing layer for Gen I Pokémon Red/Blue saves.
//
// Rules:
//  - No arbitrary offset writes.
//  - All edits go through validated setters.
//  - Always recompute required checksums after mutation.
//  - File naming (BACKUP)/(EDITED) is handled by main.cpp + FileManipulation.
// =========================================================

#include "WriteOnlyData.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace savegenie {

// =========================================================
// EditLog
// =========================================================

std::string EditLog::ToString() const {
    std::ostringstream oss;
    for (const auto& line : lines) {
        oss << line << "\n";
    }
    return oss.str();
}

// =========================================================
// Internal helpers
// =========================================================

static std::string UpperAscii(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

// =========================================================
// WriteOnlyData
// =========================================================

WriteOnlyData::WriteOnlyData(SaveBuffer& buffer) : buffer_(buffer) {}

EditMessage WriteOnlyData::SetTrainerName(const std::string& name) {
    const auto v = ValidateGen1Name(name, Gen1Layout::TrainerNameLen);
    if (!v.Ok()) return v;

    // EncodeName should write terminator + pad.
    Gen1TextCodec::EncodeName(buffer_, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen, UpperAscii(name));
    return EditMessage(EditStatus::Ok, "Trainer name set.");
}

EditMessage WriteOnlyData::SetRivalName(const std::string& name) {
    const auto v = ValidateGen1Name(name, Gen1Layout::RivalNameLen);
    if (!v.Ok()) return v;

    Gen1TextCodec::EncodeName(buffer_, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen, UpperAscii(name));
    return EditMessage(EditStatus::Ok, "Rival name set.");
}

EditMessage WriteOnlyData::SetMoney(u32 money) {
    const auto v = ValidateMoney(money);
    if (!v.Ok()) return v;

    BcdCodec::WriteBcd3(buffer_, Gen1Layout::MoneyOff, money);
    return EditMessage(EditStatus::Ok, "Money set.");
}

EditMessage WriteOnlyData::SetCoins(u16 coins) {
    const auto v = ValidateCoins(coins);
    if (!v.Ok()) return v;

    BcdCodec::WriteBcd2(buffer_, Gen1Layout::CoinsOff, coins);
    return EditMessage(EditStatus::Ok, "Coins set.");
}

EditMessage WriteOnlyData::SetBadges(u8 badgesBitfield) {
    buffer_.WriteU8(Gen1Layout::BadgesOff, badgesBitfield);
    return EditMessage(EditStatus::Ok, "Badges bitfield set.");
}

EditMessage WriteOnlyData::SetLocation(u8 mapId, u8 x, u8 y) {
    // Conservative validation: accept any mapId that isn't explicitly INVALID in your map table.
    if (Gen1MapLookup::NameFromId(mapId) == "INVALID") {
        return EditMessage(EditStatus::OutOfRange, "Map ID is invalid (per map table).");
    }

    buffer_.WriteU8(Gen1Layout::MapIdOff, mapId);
    buffer_.WriteU8(Gen1Layout::XCoordOff, x);
    buffer_.WriteU8(Gen1Layout::YCoordOff, y);

    return EditMessage(EditStatus::Ok, "Location set (MapID/X/Y).");
}

EditMessage WriteOnlyData::Apply(const EditRequest& req, EditLog* log) {
    bool any = false;

    auto run = [&](const char* label, const EditMessage& m) -> EditMessage {
        if (log) {
            std::ostringstream oss;
            oss << (m.Ok() ? "[OK] " : "[ERROR] ") << label;
            if (!m.message.empty()) oss << " - " << m.message;
            log->Add(oss.str());
        }
        return m;
    };

    if (req.newTrainerName.has_value()) {
        any = true;
        auto m = run("SetTrainerName", SetTrainerName(*req.newTrainerName));
        if (!m.Ok()) return m;
    }

    if (req.newRivalName.has_value()) {
        any = true;
        auto m = run("SetRivalName", SetRivalName(*req.newRivalName));
        if (!m.Ok()) return m;
    }

    if (req.newMoney.has_value()) {
        any = true;
        auto m = run("SetMoney", SetMoney(*req.newMoney));
        if (!m.Ok()) return m;
    }

    if (req.newCoins.has_value()) {
        any = true;
        auto m = run("SetCoins", SetCoins(*req.newCoins));
        if (!m.Ok()) return m;
    }

    if (req.newBadges.has_value()) {
        any = true;
        auto m = run("SetBadges", SetBadges(*req.newBadges));
        if (!m.Ok()) return m;
    }

    // Location requires all three.
    if (req.newMapId.has_value() || req.newX.has_value() || req.newY.has_value()) {
        any = true;
        if (!req.newMapId.has_value() || !req.newX.has_value() || !req.newY.has_value()) {
            auto m = run("SetLocation", EditMessage(EditStatus::InvalidArgument, "Location edit requires MapID, X, and Y together."));
            return m;
        }

        auto m = run("SetLocation", SetLocation(*req.newMapId, *req.newX, *req.newY));
        if (!m.Ok()) return m;
    }

    if (!any) {
        return run("Apply", EditMessage(EditStatus::Ok, "No edits requested."));
    }

    // Always fix checksums after successful edits.
    {
        auto m = run("FixChecksums", FixChecksums(log));
        if (!m.Ok()) return m;
    }

    return EditMessage(EditStatus::Ok, "Edits applied.");
}

std::vector<ItemStack> WriteOnlyData::ReadItemList(ItemListKind kind, bool includeNamesAndHex) const {
    const auto r = RegionFor(kind);
    return ReadItemListInternal(r, includeNamesAndHex);
}

EditMessage WriteOnlyData::AddOrUpdateItem(const ItemEditRequest& req, EditLog* log) {
    (void)req;
    (void)log;
    return EditMessage(EditStatus::InvalidArgument, "Item edits not implemented yet (next step).");
}

EditMessage WriteOnlyData::RemoveItem(ItemListKind kind, u8 itemId, EditLog* log) {
    (void)kind;
    (void)itemId;
    (void)log;
    return EditMessage(EditStatus::InvalidArgument, "Item edits not implemented yet (next step).");
}

EditMessage WriteOnlyData::SetItemQuantity(ItemListKind kind, u8 itemId, u8 quantity, EditLog* log) {
    (void)kind;
    (void)itemId;
    (void)quantity;
    (void)log;
    return EditMessage(EditStatus::InvalidArgument, "Item edits not implemented yet (next step).");
}

EditMessage WriteOnlyData::ApplyPokemonEdit(const PokemonEditRequest& req, EditLog* log) {
    (void)req;
    (void)log;
    return EditMessage(EditStatus::InvalidArgument, "Pokémon edits not implemented yet (placeholder).");
}

EditMessage WriteOnlyData::FixChecksums(EditLog* log) {
    (void)log;

    // MVP: edits are in Bank 1. Repair main checksum.
    Gen1Checksum::FixMain(buffer_);

    return EditMessage(EditStatus::Ok, "Main checksum repaired.");
}

EditMessage WriteOnlyData::Validate() const {
    // Minimal validation. You can expand later.
    if (buffer_.Size() != 0x8000) {
        return EditMessage(EditStatus::InvalidSave, "Save size is not 0x8000 (32KB)." );
    }

    // If checksum validation function exists, we can report mismatch.
    if (!Gen1Checksum::ValidateMain(buffer_)) {
        return EditMessage(EditStatus::ChecksumMismatch, "Main checksum is INVALID." );
    }

    return EditMessage(EditStatus::Ok, "Save is valid." );
}

std::string WriteOnlyData::DumpFullEditLog(const EditLog& log) const {
    return log.ToString();
}

std::string WriteOnlyData::DumpFullEdit(const EditLog& log) const {
    std::ostringstream oss;
    oss << "=== Save Genie Edit Summary ===\n";

    const std::string body = log.ToString();
    if (!body.empty()) {
        oss << body;
    } else {
        oss << "(no log messages)\n";
    }

    oss << "\n";
    oss << "Main Checksum: " << (Gen1Checksum::ValidateMain(buffer_) ? "VALID" : "INVALID") << "\n";

    return oss.str();
}

// =========================================================
// Private helpers
// =========================================================

bool WriteOnlyData::IsValidGen1NameChar(char c) {
    if (c == ' ') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c >= '0' && c <= '9') return true;
    return false;
}

EditMessage WriteOnlyData::ValidateGen1Name(const std::string& name, std::size_t maxLen) {
    if (name.empty()) return EditMessage(EditStatus::InvalidName, "Name cannot be empty." );
    if (name.size() > maxLen - 1) {
        std::ostringstream oss;
        oss << "Name too long (max " << (maxLen - 1) << " chars).";
        return EditMessage(EditStatus::InvalidName, oss.str());
    }

    for (char c : name) {
        if (!IsValidGen1NameChar(c)) {
            return EditMessage(EditStatus::InvalidName, "Name contains unsupported characters (use A-Z, 0-9, space)." );
        }
    }

    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::ValidateMoney(u32 money) {
    if (money > 999999u) return EditMessage(EditStatus::OutOfRange, "Money out of range (0..999999)." );
    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::ValidateCoins(u16 coins) {
    if (coins > 9999u) return EditMessage(EditStatus::OutOfRange, "Coins out of range (0..9999)." );
    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::ValidateItemId(u8 itemId, bool allowInvalidIds) {
    if (itemId == 0x00 || itemId == 0xFF) {
        return EditMessage(EditStatus::InvalidItemId, "Invalid item ID (0x00/0xFF)." );
    }

    if (!allowInvalidIds && Gen1ItemLookup::NameFromId(itemId) == "INVALID") {
        return EditMessage(EditStatus::InvalidItemId, "Invalid item ID (lookup says INVALID)." );
    }

    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::ValidateQuantity(u8 qty, u8 minQty, u8 maxQty) {
    if (qty < minQty || qty > maxQty) {
        std::ostringstream oss;
        oss << "Quantity out of range (" << static_cast<int>(minQty)
            << ".." << static_cast<int>(maxQty) << ").";
        return EditMessage(EditStatus::InvalidQuantity, oss.str());
    }

    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::WriteTrainerNameBytes(const std::string& name) {
    Gen1TextCodec::EncodeName(buffer_, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen, UpperAscii(name));
    return EditMessage(EditStatus::Ok, "");
}

EditMessage WriteOnlyData::WriteRivalNameBytes(const std::string& name) {
    Gen1TextCodec::EncodeName(buffer_, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen, UpperAscii(name));
    return EditMessage(EditStatus::Ok, "");
}

WriteOnlyData::ListRegion WriteOnlyData::RegionFor(ItemListKind kind) {
    ListRegion r;

    if (kind == ItemListKind::Bag) {
        r.base = Gen1Layout::BagItemsOff;
        r.len = Gen1Layout::BagItemsLen;
        r.countOff = Gen1Layout::BagItemsCountOff;
        r.pairsOff = Gen1Layout::BagItemsPairsOff;
        r.maxPairs = Gen1Layout::BagItemsMaxPairs;
    } else {
        r.base = Gen1Layout::PCItemBoxOff;
        r.len = Gen1Layout::PCItemBoxLen;
        r.countOff = Gen1Layout::PCItemBoxCountOff;
        r.pairsOff = Gen1Layout::PCItemBoxPairsOff;
        r.maxPairs = Gen1Layout::PCItemBoxMaxPairs;
    }

    return r;
}

std::vector<ItemStack> WriteOnlyData::ReadItemListInternal(const ListRegion& r, bool includeNamesAndHex) const {
    std::vector<ItemStack> out;

    buffer_.RequireRange(r.base, r.len);

    const u8 rawCount = buffer_.ReadU8(r.countOff);
    const int count = std::clamp<int>(static_cast<int>(rawCount), 0, r.maxPairs);

    std::size_t off = r.pairsOff;
    for (int i = 0; i < count; ++i) {
        buffer_.RequireRange(off, 2);

        const u8 itemId = buffer_.ReadU8(off + 0);
        const u8 qty    = buffer_.ReadU8(off + 1);

        if (itemId == 0xFF) break;

        ItemStack it;
        it.itemId = itemId;
        it.quantity = qty;

        if (includeNamesAndHex) {
            it.itemName = Gen1ItemLookup::NameFromId(itemId);
            it.itemHex  = Gen1ItemLookup::ItemHex[static_cast<std::size_t>(itemId)];
        }

        out.push_back(std::move(it));
        off += 2;
    }

    return out;
}

EditMessage WriteOnlyData::WriteItemListInternal(const ListRegion& r, const std::vector<ItemStack>& items, EditLog* log) {
    (void)r;
    (void)items;
    (void)log;
    return EditMessage(EditStatus::InvalidArgument, "WriteItemListInternal not implemented yet (next step)." );
}

} // namespace savegenie
