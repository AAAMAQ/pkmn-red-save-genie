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
