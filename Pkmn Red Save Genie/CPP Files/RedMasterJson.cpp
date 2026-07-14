//
//  RedMasterJson.cpp
//  Pkmn Red Save Genie
//
//  Draft .red.json implementation focused first on lossless reconstruction.
//

#include "RedMasterJson.hpp"

#include "RedTestExports.hpp"

#include <CommonCrypto/CommonDigest.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace savegenie {
namespace {

static constexpr const char* kFormat = "pkmn-red-master-save";
static constexpr const char* kSchemaVersion = "0.1.0";
static constexpr const char* kHexEncoding = "hex_uppercase_continuous";

std::string HexSize(std::size_t value, int width = 0) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex;
    if (width > 0) {
        oss << std::setw(width) << std::setfill('0');
    }
    oss << value << std::dec;
    return oss.str();
}

std::string UtcTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm {};
#if defined(_WIN32)
    gmtime_s(&tm, &now);
#else
    gmtime_r(&now, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

FileManipulation::Bytes SliceBytes(const FileManipulation::Bytes& bytes, std::size_t off, std::size_t len) {
    if (off > bytes.size() || len > bytes.size() - off) {
        throw std::out_of_range("SliceBytes range out of bounds");
    }
    return FileManipulation::Bytes(bytes.begin() + static_cast<std::ptrdiff_t>(off),
                                   bytes.begin() + static_cast<std::ptrdiff_t>(off + len));
}

bool FileExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) && !ec;
}

std::string CollisionSafePathWithPrefix(const std::filesystem::path& dir,
                                        const std::string& prefix,
                                        const std::string& filename) {
    for (int i = 1; i < 10000; ++i) {
        std::ostringstream candidateName;
        if (i == 1) {
            candidateName << prefix << filename;
        } else {
            std::string numberedPrefix = prefix;
            const std::size_t close = numberedPrefix.find(']');
            if (close != std::string::npos) {
                numberedPrefix.insert(close, " " + std::to_string(i));
            } else {
                numberedPrefix += std::to_string(i) + " ";
            }
            candidateName << numberedPrefix << filename;
        }

        const std::filesystem::path candidate = dir / candidateName.str();
        if (!FileExists(candidate)) {
            return candidate.string();
        }
    }

    throw std::runtime_error("Could not create collision-safe output path for " + filename);
}

struct JsonValue {
    enum class Type { Null, Bool, Number, String, Object, Array };

    Type type = Type::Null;
    bool boolValue = false;
    std::string text;
    std::map<std::string, JsonValue> object;
    std::vector<JsonValue> array;
};

class JsonParser {
public:
    explicit JsonParser(std::string_view input) : input_(input) {}

    bool Parse(JsonValue* out, std::string* error) {
        SkipWs();
        if (!ParseValue(out, error)) return false;
        SkipWs();
        if (pos_ != input_.size()) {
            SetError(error, "Unexpected trailing data after JSON document");
            return false;
        }
        return true;
    }

private:
    void SkipWs() {
        while (pos_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[pos_]))) {
            ++pos_;
        }
    }

    bool Match(std::string_view s) {
        if (input_.substr(pos_, s.size()) == s) {
            pos_ += s.size();
            return true;
        }
        return false;
    }

    void SetError(std::string* error, const std::string& message) const {
        if (error) {
            std::ostringstream oss;
            oss << message << " at byte " << pos_;
            *error = oss.str();
        }
    }

    bool ParseValue(JsonValue* out, std::string* error) {
        SkipWs();
        if (pos_ >= input_.size()) {
            SetError(error, "Unexpected end of JSON input");
            return false;
        }

        const char c = input_[pos_];
        if (c == '"') return ParseString(out, error);
        if (c == '{') return ParseObject(out, error);
        if (c == '[') return ParseArray(out, error);
        if (c == 't') {
            if (!Match("true")) {
                SetError(error, "Invalid literal");
                return false;
            }
            out->type = JsonValue::Type::Bool;
            out->boolValue = true;
            return true;
        }
        if (c == 'f') {
            if (!Match("false")) {
                SetError(error, "Invalid literal");
                return false;
            }
            out->type = JsonValue::Type::Bool;
            out->boolValue = false;
            return true;
        }
        if (c == 'n') {
            if (!Match("null")) {
                SetError(error, "Invalid literal");
                return false;
            }
            out->type = JsonValue::Type::Null;
            return true;
        }
        if (c == '-' || (c >= '0' && c <= '9')) return ParseNumber(out, error);

        SetError(error, "Unexpected JSON token");
        return false;
    }

    bool ParseString(JsonValue* out, std::string* error) {
        if (input_[pos_] != '"') {
            SetError(error, "Expected string");
            return false;
        }
        ++pos_;
        std::string value;
        while (pos_ < input_.size()) {
            const char c = input_[pos_++];
            if (c == '"') {
                out->type = JsonValue::Type::String;
                out->text = std::move(value);
                return true;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                SetError(error, "Unescaped control character in string");
                return false;
            }
            if (c != '\\') {
                value.push_back(c);
                continue;
            }
            if (pos_ >= input_.size()) {
                SetError(error, "Unfinished string escape");
                return false;
            }
            const char esc = input_[pos_++];
            switch (esc) {
                case '"': value.push_back('"'); break;
                case '\\': value.push_back('\\'); break;
                case '/': value.push_back('/'); break;
                case 'b': value.push_back('\b'); break;
                case 'f': value.push_back('\f'); break;
                case 'n': value.push_back('\n'); break;
                case 'r': value.push_back('\r'); break;
                case 't': value.push_back('\t'); break;
                case 'u':
                    if (pos_ + 4 > input_.size()) {
                        SetError(error, "Incomplete unicode escape");
                        return false;
                    }
                    // The exporter writes ASCII-only escaped strings for required
                    // fields. Preserve uncommon unicode escapes as '?' for parser
                    // completeness without pretending to normalize text.
                    for (int i = 0; i < 4; ++i) {
                        if (!std::isxdigit(static_cast<unsigned char>(input_[pos_ + i]))) {
                            SetError(error, "Invalid unicode escape");
                            return false;
                        }
                    }
                    pos_ += 4;
                    value.push_back('?');
                    break;
                default:
                    SetError(error, "Invalid string escape");
                    return false;
            }
        }

        SetError(error, "Unterminated string");
        return false;
    }

    bool ParseNumber(JsonValue* out, std::string* error) {
        const std::size_t start = pos_;
        if (input_[pos_] == '-') ++pos_;
        if (pos_ >= input_.size()) {
            SetError(error, "Invalid number");
            return false;
        }
        if (input_[pos_] == '0') {
            ++pos_;
        } else if (input_[pos_] >= '1' && input_[pos_] <= '9') {
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        } else {
            SetError(error, "Invalid number");
            return false;
        }
        if (pos_ < input_.size() && input_[pos_] == '.') {
            ++pos_;
            if (pos_ >= input_.size() || !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                SetError(error, "Invalid number fraction");
                return false;
            }
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        }
        if (pos_ < input_.size() && (input_[pos_] == 'e' || input_[pos_] == 'E')) {
            ++pos_;
            if (pos_ < input_.size() && (input_[pos_] == '+' || input_[pos_] == '-')) ++pos_;
            if (pos_ >= input_.size() || !std::isdigit(static_cast<unsigned char>(input_[pos_]))) {
                SetError(error, "Invalid number exponent");
                return false;
            }
            while (pos_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_]))) ++pos_;
        }

        out->type = JsonValue::Type::Number;
        out->text = std::string(input_.substr(start, pos_ - start));
        return true;
    }

    bool ParseObject(JsonValue* out, std::string* error) {
        ++pos_;
        out->type = JsonValue::Type::Object;
        out->object.clear();
        SkipWs();
        if (pos_ < input_.size() && input_[pos_] == '}') {
            ++pos_;
            return true;
        }
        while (true) {
            SkipWs();
            JsonValue key;
            if (!ParseString(&key, error)) return false;
            SkipWs();
            if (pos_ >= input_.size() || input_[pos_] != ':') {
                SetError(error, "Expected ':' after object key");
                return false;
            }
            ++pos_;
            JsonValue value;
            if (!ParseValue(&value, error)) return false;
            out->object[key.text] = std::move(value);
            SkipWs();
            if (pos_ >= input_.size()) {
                SetError(error, "Unterminated object");
                return false;
            }
            if (input_[pos_] == '}') {
                ++pos_;
                return true;
            }
            if (input_[pos_] != ',') {
                SetError(error, "Expected ',' or '}' in object");
                return false;
            }
            ++pos_;
        }
    }

    bool ParseArray(JsonValue* out, std::string* error) {
        ++pos_;
        out->type = JsonValue::Type::Array;
        out->array.clear();
        SkipWs();
        if (pos_ < input_.size() && input_[pos_] == ']') {
            ++pos_;
            return true;
        }
        while (true) {
            JsonValue value;
            if (!ParseValue(&value, error)) return false;
            out->array.push_back(std::move(value));
            SkipWs();
            if (pos_ >= input_.size()) {
                SetError(error, "Unterminated array");
                return false;
            }
            if (input_[pos_] == ']') {
                ++pos_;
                return true;
            }
            if (input_[pos_] != ',') {
                SetError(error, "Expected ',' or ']' in array");
                return false;
            }
            ++pos_;
        }
    }

    std::string_view input_;
    std::size_t pos_ = 0;
};

const JsonValue* ObjectField(const JsonValue& object, const std::string& key) {
    if (object.type != JsonValue::Type::Object) return nullptr;
    const auto it = object.object.find(key);
    if (it == object.object.end()) return nullptr;
    return &it->second;
}

bool RequireObject(const JsonValue& object, const std::string& key, const JsonValue** out, std::vector<std::string>* errors) {
    const JsonValue* value = ObjectField(object, key);
    if (!value || value->type != JsonValue::Type::Object) {
        if (errors) errors->push_back("Missing or invalid object field: " + key);
        return false;
    }
    *out = value;
    return true;
}

bool RequireString(const JsonValue& object, const std::string& key, std::string* out, std::vector<std::string>* errors) {
    const JsonValue* value = ObjectField(object, key);
    if (!value || value->type != JsonValue::Type::String) {
        if (errors) errors->push_back("Missing or invalid string field: " + key);
        return false;
    }
    *out = value->text;
    return true;
}

bool RequireSize(const JsonValue& object, const std::string& key, std::size_t* out, std::vector<std::string>* errors) {
    const JsonValue* value = ObjectField(object, key);
    if (!value || value->type != JsonValue::Type::Number) {
        if (errors) errors->push_back("Missing or invalid number field: " + key);
        return false;
    }
    try {
        std::size_t consumed = 0;
        const unsigned long long parsed = std::stoull(value->text, &consumed, 10);
        if (consumed != value->text.size()) {
            if (errors) errors->push_back("Invalid integer field: " + key);
            return false;
        }
        *out = static_cast<std::size_t>(parsed);
        return true;
    } catch (...) {
        if (errors) errors->push_back("Invalid integer field: " + key);
        return false;
    }
}

std::string RawHexAt(const SaveBuffer& save, std::size_t off, std::size_t len) {
    return RedBytesToCanonicalHex(save.Slice(off, len));
}

std::string FieldMeta(std::size_t off, std::size_t len, const std::string& encoding, const std::string& confidence = "verified") {
    std::ostringstream json;
    json << "\"offset\": \"" << HexSize(off, 4) << "\", "
         << "\"length\": " << len << ", "
         << "\"encoding\": \"" << encoding << "\", "
         << "\"confidence\": \"" << confidence << "\"";
    return json.str();
}

std::string StatusName(u8 status) {
    if (status == 0) return "healthy";
    if (status & 0x07) return "sleep";
    if (status & 0x08) return "poisoned";
    if (status & 0x10) return "burned";
    if (status & 0x20) return "frozen";
    if (status & 0x40) return "paralyzed";
    return "unknown_status_bits";
}

int DexNoToInt(const std::string& dexNo) {
    try {
        std::size_t consumed = 0;
        const int value = std::stoi(dexNo, &consumed, 10);
        return (consumed == dexNo.size()) ? value : 0;
    } catch (...) {
        return 0;
    }
}

std::string ItemConversionStatus(const std::string& itemName) {
    static const std::vector<std::string> keyOrProgressItems = {
        "BICYCLE", "SURFBOARD", "POKEDEX", "BOULDERBADGE", "CASCADEBADGE",
        "THUNDERBADGE", "RAINBOWBADGE", "SOULBADGE", "MARSHBADGE",
        "VOLCANOBADGE", "EARTHBADGE", "OLD AMBER", "DOME FOSSIL",
        "HELIX FOSSIL", "SECRET KEY", "BIKE VOUCHER", "CARD KEY",
        "S.S.TICKET", "GOLD TEETH", "COIN CASE", "OAKS PARCEL",
        "ITEMFINDER", "SILPH SCOPE", "POKE FLUTE", "LIFT KEY",
        "EXP.ALL", "OLD ROD", "GOOD ROD", "SUPER ROD"
    };
    if (itemName.empty() || itemName == "INVALID" || itemName == "UNKNOWN") return "unsupported";
    if (itemName.rfind("TM", 0) == 0 || itemName.rfind("HM", 0) == 0) return "semantic_translation";
    if (std::find(keyOrProgressItems.begin(), keyOrProgressItems.end(), itemName) != keyOrProgressItems.end()) {
        return "policy_required";
    }
    return "direct_by_name";
}

std::string ItemConversionPolicy(const std::string& itemName) {
    const std::string status = ItemConversionStatus(itemName);
    if (status == "direct_by_name") return "map_to_fire_red_item_with_same_semantic_name";
    if (status == "semantic_translation") return "map_tm_hm_by_taught_move_not_numeric_item_id";
    if (status == "policy_required") return "preserve_as_inventory_or_story_state_based_on_fire_red_progress_policy";
    return "reject_or_report_unsupported_item";
}

int DraftIvFromDv(u8 dv) {
    return static_cast<int>(dv) * 2 + 1;
}

struct DraftEvSpread {
    int hp = 0;
    int attack = 0;
    int defense = 0;
    int speed = 0;
    int specialAttack = 0;
    int specialDefense = 0;
    int uncappedTotal = 0;
    int cappedTotal = 0;
    bool capped = false;
};

DraftEvSpread BuildDraftEvSpread(const PokemonMon& mon) {
    DraftEvSpread ev;
    ev.hp = std::min(255, static_cast<int>(mon.statExpHP / 256));
    ev.attack = std::min(255, static_cast<int>(mon.statExpAtk / 256));
    ev.defense = std::min(255, static_cast<int>(mon.statExpDef / 256));
    ev.speed = std::min(255, static_cast<int>(mon.statExpSpd / 256));
    ev.specialAttack = std::min(255, static_cast<int>(mon.statExpSpc / 256));
    ev.specialDefense = std::min(255, static_cast<int>(mon.statExpSpc / 256));
    ev.uncappedTotal = ev.hp + ev.attack + ev.defense + ev.speed + ev.specialAttack + ev.specialDefense;
    ev.cappedTotal = ev.uncappedTotal;
    if (ev.uncappedTotal <= 510) return ev;

    ev.capped = true;
    std::array<int*, 6> values = { &ev.hp, &ev.attack, &ev.defense, &ev.speed, &ev.specialAttack, &ev.specialDefense };
    std::array<int, 6> original = { ev.hp, ev.attack, ev.defense, ev.speed, ev.specialAttack, ev.specialDefense };
    int scaledTotal = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        *values[i] = (original[i] * 510) / ev.uncappedTotal;
        scaledTotal += *values[i];
    }
    int remaining = 510 - scaledTotal;
    for (std::size_t i = 0; remaining > 0 && i < values.size(); ++i) {
        if (*values[i] < 255 && original[i] > 0) {
            ++(*values[i]);
            --remaining;
        }
    }
    ev.cappedTotal = ev.hp + ev.attack + ev.defense + ev.speed + ev.specialAttack + ev.specialDefense;
    return ev;
}

u32 DeterministicPokemonSeed(const PokemonMon& mon) {
    u32 hash = 2166136261u;
    auto mix = [&hash](u32 value) {
        hash ^= value;
        hash *= 16777619u;
    };
    mix(mon.speciesId);
    mix(mon.otIdNo);
    mix(mon.level);
    mix(mon.expPoints);
    for (const char c : mon.nickname) mix(static_cast<u8>(c));
    for (const char c : mon.otName) mix(static_cast<u8>(c));
    for (const PokemonMove& move : mon.moves) mix(move.moveId);
    return hash;
}

void WritePokemonConversionJson(std::ostringstream& json, const PokemonMon& mon, const std::string& indent) {
    const int dexNo = DexNoToInt(mon.dexNo);
    const DraftEvSpread ev = BuildDraftEvSpread(mon);
    const u32 seed = DeterministicPokemonSeed(mon);
    json << indent << "\"conversion\": {\n";
    json << indent << "  \"classification\": \"" << (dexNo >= 1 && dexNo <= 151 ? "direct_transfer" : "unsupported_or_policy_required") << "\",\n";
    json << indent << "  \"sourceGame\": \"pokemon_red\",\n";
    json << indent << "  \"targetGame\": \"pokemon_firered\",\n";
    json << indent << "  \"speciesMapping\": { \"sourceInternalId\": " << static_cast<int>(mon.speciesId)
         << ", \"nationalDexNumber\": " << dexNo
         << ", \"targetBasis\": \"national_dex_number_not_gen_i_internal_id\", \"status\": \""
         << (dexNo >= 1 && dexNo <= 151 ? "translated" : "unsupported") << "\" },\n";
    json << indent << "  \"heldItemPolicy\": { \"targetHeldItem\": \"NONE\", \"classification\": \"initialize_to_safe_default\", \"reason\": \"Gen I Pokemon do not store held items\" },\n";
    json << indent << "  \"dvToIvPolicy\": { \"name\": \"draft_dv_times_2_plus_1\", \"hp\": " << DraftIvFromDv(mon.dvHP)
         << ", \"attack\": " << DraftIvFromDv(mon.dvAtk)
         << ", \"defense\": " << DraftIvFromDv(mon.dvDef)
         << ", \"speed\": " << DraftIvFromDv(mon.dvSpd)
         << ", \"specialAttack\": " << DraftIvFromDv(mon.dvSpc)
         << ", \"specialDefense\": " << DraftIvFromDv(mon.dvSpc)
         << ", \"confidence\": \"policy_draft\" },\n";
    json << indent << "  \"statExperienceToEvPolicy\": { \"name\": \"draft_divide_by_256_split_special_and_cap_510\", \"hp\": " << ev.hp
         << ", \"attack\": " << ev.attack
         << ", \"defense\": " << ev.defense
         << ", \"speed\": " << ev.speed
         << ", \"specialAttack\": " << ev.specialAttack
         << ", \"specialDefense\": " << ev.specialDefense
         << ", \"uncappedTotal\": " << ev.uncappedTotal
         << ", \"cappedTotal\": " << ev.cappedTotal
         << ", \"capped\": " << (ev.capped ? "true" : "false")
         << ", \"classification\": \"approximate_deterministically\" },\n";
    json << indent << "  \"generatedFieldsPolicy\": {\n";
    json << indent << "    \"personalitySeed\": \"" << HexSize(seed, 8) << "\",\n";
    json << indent << "    \"personalityValue\": \"derive_later_from_seed_when_gen_iii_codec_exists\",\n";
    json << indent << "    \"nature\": \"derive_deterministically_from_personality_seed\",\n";
    json << indent << "    \"gender\": \"derive_from_fire_red_species_gender_ratio_and_personality\",\n";
    json << indent << "    \"ability\": \"derive_from_fire_red_species_ability_rules\",\n";
    json << indent << "    \"friendship\": \"initialize_to_fire_red_species_default_or_policy_value\",\n";
    json << indent << "    \"metLocation\": \"conversion_policy_required\",\n";
    json << indent << "    \"metLevel\": " << static_cast<int>(mon.level) << ",\n";
    json << indent << "    \"originGame\": \"pokemon_red_virtualized_source\",\n";
    json << indent << "    \"pokeBall\": \"poke_ball_default_policy\",\n";
    json << indent << "    \"language\": \"english_default_policy\",\n";
    json << indent << "    \"ribbons\": \"none_safe_default\"\n";
    json << indent << "  },\n";
    json << indent << "  \"warnings\": [";
    bool wroteWarning = false;
    if (ev.capped) {
        json << "\"Gen I Stat Experience exceeds the Gen III 510 EV cap after draft scaling\"";
        wroteWarning = true;
    }
    if (mon.stats.status != 0) {
        if (wroteWarning) json << ", ";
        json << "\"Persistent Gen I party status requires target-side policy; boxed Pokemon status is not stored the same way\"";
    }
    json << "]\n";
    json << indent << "}";
}

std::string EventDescription(const std::string& eventName) {
    std::string name = eventName;
    if (name.rfind("EVENT_", 0) == 0) {
        name = name.substr(6);
    }

    const bool beat = name.rfind("BEAT_", 0) == 0;
    const bool got = name.rfind("GOT_", 0) == 0;
    const bool defeated = name.rfind("DEFEATED_", 0) == 0;

    if (beat) name = name.substr(5);
    else if (got) name = name.substr(4);
    else if (defeated) name = name.substr(9);

    std::replace(name.begin(), name.end(), '_', ' ');
    std::string out;
    bool newWord = true;
    for (char c : name) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            out.push_back(' ');
            newWord = true;
        } else if (newWord) {
            out.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            newWord = false;
        } else {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }

    if (beat || defeated) return out + " has been defeated";
    if (got) return out + " obtained";
    return out;
}

std::string EventPersistence(const std::string& eventName) {
    if (eventName.find("AUTOWALKED") != std::string::npos ||
        eventName.find("ROOM") != std::string::npos ||
        eventName.find("SAFARI") != std::string::npos ||
        eventName.find("LOCK_DOOR") != std::string::npos) {
        return "temporary";
    }
    if (eventName.find("BEAT_") != std::string::npos ||
        eventName.find("GOT_") != std::string::npos ||
        eventName.find("DEFEATED_") != std::string::npos) {
        return "persistent";
    }
    return "unknown";
}

std::string EventCategoryFromName(const std::string& eventName) {
    if (eventName.find("BROCK") != std::string::npos ||
        eventName.find("MISTY") != std::string::npos ||
        eventName.find("LT_SURGE") != std::string::npos ||
        eventName.find("ERIKA") != std::string::npos ||
        eventName.find("KOGA") != std::string::npos ||
        eventName.find("BLAINE") != std::string::npos ||
        eventName.find("SABRINA") != std::string::npos ||
        eventName.find("GIOVANNI") != std::string::npos) {
        return "gym_or_major_battle";
    }
    if (eventName.find("ARTICUNO") != std::string::npos ||
        eventName.find("ZAPDOS") != std::string::npos ||
        eventName.find("MOLTRES") != std::string::npos ||
        eventName.find("MEWTWO") != std::string::npos) {
        return "legendary_or_static_battle";
    }
    if (eventName.find("BEAT_") != std::string::npos) return "trainer_or_battle";
    if (eventName.find("GOT_") != std::string::npos) return "item_or_reward";
    return "event";
}

void WriteBadgeArray(std::ostringstream& json, u8 badges) {
    static const char* kBadgeNames[8] = {
        "Boulder Badge", "Cascade Badge", "Thunder Badge", "Rainbow Badge",
        "Soul Badge", "Marsh Badge", "Volcano Badge", "Earth Badge"
    };

    json << "      \"badges\": [\n";
    for (int i = 0; i < 8; ++i) {
        json << "        { \"index\": " << (i + 1)
             << ", \"name\": \"" << kBadgeNames[i] << "\""
             << ", \"owned\": " << ((badges & static_cast<u8>(1u << i)) ? "true" : "false")
             << ", \"byteOffset\": \"" << HexSize(Gen1Layout::BadgesOff, 4) << "\""
             << ", \"bitIndex\": " << i
             << ", \"confidence\": \"verified\" }";
        if (i != 7) json << ",";
        json << "\n";
    }
    json << "      ]";
}

void WriteMovesArray(std::ostringstream& json,
                     const SaveBuffer& save,
                     const PokemonMon& mon,
                     std::size_t structOff,
                     const std::string& indent) {
    json << indent << "\"moves\": [\n";
    for (std::size_t i = 0; i < mon.moves.size(); ++i) {
        const PokemonMove& move = mon.moves[i];
        const std::size_t moveOff = structOff + 0x08 + i;
        const std::size_t ppOff = structOff + 0x1D + i;
        const u8 rawPP = save.ReadU8(ppOff);
        json << indent << "  {\n";
        json << indent << "    \"slot\": " << (i + 1) << ",\n";
        json << indent << "    \"move\": { \"name\": \"" << JsonEscape(move.moveName)
             << "\", \"id\": " << static_cast<int>(move.moveId)
             << ", \"rawIdHex\": \"" << HexSize(move.moveId, 2) << "\" },\n";
        json << indent << "    \"pp\": { \"current\": " << static_cast<int>(move.ppCurrent)
             << ", \"maximum\": " << static_cast<int>(move.ppMax)
             << ", \"ppUps\": " << static_cast<int>((rawPP >> 6) & 0x03)
             << ", \"rawByte\": \"" << HexSize(rawPP, 2)
             << "\", \"note\": \"maximum PP is currently a placeholder equal to current PP\" },\n";
        json << indent << "    \"source\": { \"moveOffset\": \"" << HexSize(moveOff, 4)
             << "\", \"ppOffset\": \"" << HexSize(ppOff, 4) << "\" }\n";
        json << indent << "  }";
        if (i + 1 < mon.moves.size()) json << ",";
        json << "\n";
    }
    json << indent << "]";
}

void WritePokemonMonJson(std::ostringstream& json,
                         const SaveBuffer& save,
                         const PokemonMon& mon,
                         std::size_t structOff,
                         std::size_t structLen,
                         std::size_t speciesListOff,
                         std::size_t otNameOff,
                         std::size_t nicknameOff,
                         bool partyStats,
                         const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"position\": " << mon.position << ",\n";
    json << indent << "  \"species\": {\n";
    json << indent << "    \"name\": \"" << JsonEscape(mon.speciesName) << "\",\n";
    json << indent << "    \"internalId\": " << static_cast<int>(mon.speciesId) << ",\n";
    json << indent << "    \"internalIdHex\": \"" << HexSize(mon.speciesId, 2) << "\",\n";
    json << indent << "    \"nationalDexNumber\": " << DexNoToInt(mon.dexNo) << ",\n";
    json << indent << "    \"sourceOffset\": \"" << HexSize(speciesListOff, 4) << "\"\n";
    json << indent << "  },\n";
    json << indent << "  \"nickname\": { \"value\": \"" << JsonEscape(mon.nickname)
         << "\", \"losslessValue\": \"" << JsonEscape(Gen1TextCodec::DecodeNameLossless(
                save, nicknameOff, Gen1Layout::Gen1NameLen))
         << "\", \"rawHex\": \"" << RawHexAt(save, nicknameOff, Gen1Layout::Gen1NameLen)
         << "\", " << FieldMeta(nicknameOff, Gen1Layout::Gen1NameLen, "gen1_text") << " },\n";
    json << indent << "  \"originalTrainer\": { \"name\": \"" << JsonEscape(mon.otName)
         << "\", \"nameLossless\": \"" << JsonEscape(Gen1TextCodec::DecodeNameLossless(
                save, otNameOff, Gen1Layout::Gen1NameLen))
         << "\", \"idNo\": " << mon.otIdNo
         << ", \"nameRawHex\": \"" << RawHexAt(save, otNameOff, Gen1Layout::Gen1NameLen)
         << "\", \"nameOffset\": \"" << HexSize(otNameOff, 4) << "\" },\n";
    json << indent << "  \"level\": " << static_cast<int>(mon.level) << ",\n";
    json << indent << "  \"experience\": " << mon.expPoints << ",\n";
    json << indent << "  \"types\": { \"primaryId\": " << static_cast<int>(mon.type1)
         << ", \"secondaryId\": " << static_cast<int>(mon.type2)
         << ", \"confidence\": \"verified_stored_fields\" },\n";
    json << indent << "  \"catchRate\": { \"value\": " << static_cast<int>(mon.catchRate)
         << ", \"rawByte\": \"" << HexSize(mon.catchRate, 2)
         << "\", \"confidence\": \"verified_stored_field\" },\n";
    json << indent << "  \"status\": { \"rawByte\": \"" << HexSize(mon.stats.status, 2)
         << "\", \"name\": \"" << StatusName(mon.stats.status)
         << "\", \"confidence\": \"verified_stored_field\" },\n";
    json << indent << "  \"stats\": {\n";
    json << indent << "    \"hpCurrent\": " << mon.stats.hpCurrent << ",\n";
    json << indent << "    \"hpMax\": " << mon.stats.hpMax << ",\n";
    json << indent << "    \"attack\": " << mon.stats.attack << ",\n";
    json << indent << "    \"defense\": " << mon.stats.defense << ",\n";
    json << indent << "    \"speed\": " << mon.stats.speed << ",\n";
    json << indent << "    \"special\": " << mon.stats.special << ",\n";
    json << indent << "    \"interpretation\": \""
         << (partyStats ? "live_party_stats" : "current_hp_stored_other_battle_stats_not_stored")
         << "\"\n";
    json << indent << "  },\n";
    WriteMovesArray(json, save, mon, structOff, indent + "  ");
    json << ",\n";
    json << indent << "  \"dvs\": { \"hp\": " << static_cast<int>(mon.dvHP)
         << ", \"attack\": " << static_cast<int>(mon.dvAtk)
         << ", \"defense\": " << static_cast<int>(mon.dvDef)
         << ", \"speed\": " << static_cast<int>(mon.dvSpd)
         << ", \"special\": " << static_cast<int>(mon.dvSpc)
         << ", \"packing\": \"atk/def and speed/special nibbles; HP derived from low bits\" },\n";
    json << indent << "  \"statExperience\": { \"hp\": " << mon.statExpHP
         << ", \"attack\": " << mon.statExpAtk
         << ", \"defense\": " << mon.statExpDef
         << ", \"speed\": " << mon.statExpSpd
         << ", \"special\": " << mon.statExpSpc << " },\n";
    WritePokemonConversionJson(json, mon, indent + "  ");
    json << ",\n";
    json << indent << "  \"sourceRange\": { \"structOffset\": \"" << HexSize(structOff, 4)
         << "\", \"structLength\": " << structLen
         << ", \"rawStructureHex\": \"" << RawHexAt(save, structOff, structLen)
         << "\", \"reconstructionPolicy\": \"preserve-original-bytes\" }\n";
    json << indent << "}";
}

double AverageLevel(const PokemonBox& box) {
    if (box.pokemon.empty()) return 0.0;
    int sum = 0;
    for (const PokemonMon& mon : box.pokemon) sum += static_cast<int>(mon.level);
    return static_cast<double>(sum) / static_cast<double>(box.pokemon.size());
}

void WritePokemonBoxCanonical(std::ostringstream& json,
                              const SaveBuffer& save,
                              const PokemonBox& box,
                              std::size_t base,
                              bool isParty,
                              const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"boxNumber\": ";
    if (box.boxNumber >= 0) json << box.boxNumber;
    else json << "null";
    json << ",\n";
    json << indent << "  \"label\": \"" << JsonEscape(box.label) << "\",\n";
    json << indent << "  \"count\": " << box.pokemonCount << ",\n";
    json << indent << "  \"averageLevel\": " << std::fixed << std::setprecision(2) << AverageLevel(box) << std::defaultfloat << ",\n";
    json << indent << "  \"sourceRange\": { \"start\": \"" << HexSize(base, 4)
         << "\", \"length\": " << (isParty ? Gen1Layout::PartyBlockLen : Gen1Layout::BoxBlockSize)
         << ", \"reconstructionPolicy\": \"preserve-original-bytes\" },\n";
    json << indent << "  \"pokemon\": [\n";
    for (std::size_t i = 0; i < box.pokemon.size(); ++i) {
        const PokemonMon& mon = box.pokemon[i];
        std::size_t structOff = 0;
        std::size_t structLen = 0;
        std::size_t speciesOff = 0;
        std::size_t otOff = 0;
        std::size_t nickOff = 0;
        if (isParty) {
            structOff = Gen1Layout::PartyStructsOff + i * Gen1Layout::PartyStructSize;
            structLen = Gen1Layout::PartyStructSize;
            speciesOff = Gen1Layout::PartySpeciesOff + i;
            otOff = Gen1Layout::PartyOTNamesOff + i * Gen1Layout::Gen1NameLen;
            nickOff = Gen1Layout::PartyNicknamesOff + i * Gen1Layout::Gen1NameLen;
        } else {
            structOff = base + Gen1Layout::BoxStructsRel + i * Gen1Layout::BoxStructSize;
            structLen = Gen1Layout::BoxStructSize;
            speciesOff = base + Gen1Layout::BoxSpeciesRel + i;
            otOff = base + Gen1Layout::BoxOTNamesRel + i * Gen1Layout::Gen1NameLen;
            nickOff = base + Gen1Layout::BoxNicknamesRel + i * Gen1Layout::Gen1NameLen;
        }
        WritePokemonMonJson(json, save, mon, structOff, structLen, speciesOff, otOff, nickOff, isParty, indent + "    ");
        if (i + 1 < box.pokemon.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteBagSummaryJson(std::ostringstream& json,
                         const BagSummary& summary,
                         std::size_t countOff,
                         std::size_t pairsOff,
                         const std::string& listName,
                         const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"name\": \"" << listName << "\",\n";
    json << indent << "  \"count\": " << summary.itemCount << ",\n";
    json << indent << "  \"countOffset\": \"" << HexSize(countOff, 4) << "\",\n";
    json << indent << "  \"items\": [\n";
    for (std::size_t i = 0; i < summary.items.size(); ++i) {
        const BagItem& item = summary.items[i];
        const std::size_t itemOff = pairsOff + i * 2;
        json << indent << "    { \"slot\": " << (i + 1)
             << ", \"item\": { \"name\": \"" << JsonEscape(item.itemName)
             << "\", \"id\": " << static_cast<int>(item.itemId)
             << ", \"hex\": \"" << JsonEscape(item.itemHex) << "\" }"
             << ", \"quantity\": " << static_cast<int>(item.quantity)
             << ", \"conversion\": { \"classification\": \"" << ItemConversionStatus(item.itemName)
             << "\", \"targetPolicy\": \"" << ItemConversionPolicy(item.itemName)
             << "\", \"targetBasis\": \"semantic_item_name_not_gen_i_item_id\" }"
             << ", \"itemOffset\": \"" << HexSize(itemOff, 4)
             << "\", \"quantityOffset\": \"" << HexSize(itemOff + 1, 4)
             << "\", \"confidence\": \"verified\" }";
        if (i + 1 < summary.items.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

bool VectorContainsInt(const std::vector<int>& values, int needle) {
    return std::find(values.begin(), values.end(), needle) != values.end();
}

void WritePokedexJson(std::ostringstream& json, const PokedexSummary& pokedex, const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"ownedCount\": " << pokedex.ownedCount << ",\n";
    json << indent << "  \"seenCount\": " << pokedex.seenCount << ",\n";
    json << indent << "  \"ownedDexNumbers\": [";
    for (std::size_t i = 0; i < pokedex.ownedDexNos.size(); ++i) {
        json << pokedex.ownedDexNos[i];
        if (i + 1 < pokedex.ownedDexNos.size()) json << ", ";
    }
    json << "],\n";
    json << indent << "  \"seenDexNumbers\": [";
    for (std::size_t i = 0; i < pokedex.seenDexNos.size(); ++i) {
        json << pokedex.seenDexNos[i];
        if (i + 1 < pokedex.seenDexNos.size()) json << ", ";
    }
    json << "],\n";
    json << indent << "  \"species\": [\n";
    for (int dexNo = 1; dexNo <= 151; ++dexNo) {
        const int bitIndex = dexNo - 1;
        const int byteIndex = bitIndex / 8;
        const int bitInByte = bitIndex % 8;
        const int internalId = Gen1SpeciesLookup::PokeDex[dexNo];
        const std::string name = (internalId >= 0)
            ? Gen1SpeciesLookup::NameFromId(static_cast<u8>(internalId))
            : "INVALID";
        json << indent << "    { \"nationalDexNumber\": " << dexNo
             << ", \"speciesName\": \"" << JsonEscape(name) << "\""
             << ", \"internalId\": " << internalId
             << ", \"owned\": " << (VectorContainsInt(pokedex.ownedDexNos, dexNo) ? "true" : "false")
             << ", \"seen\": " << (VectorContainsInt(pokedex.seenDexNos, dexNo) ? "true" : "false")
             << ", \"fireRedMapping\": { \"targetBasis\": \"national_dex_number\", \"targetNationalDexNumber\": " << dexNo
             << ", \"classification\": \"direct_transfer\", \"confidence\": \"verified\" }"
             << ", \"ownedByteOffset\": \"" << HexSize(Gen1Layout::PokedexOwnedOff + static_cast<std::size_t>(byteIndex), 4)
             << "\", \"seenByteOffset\": \"" << HexSize(Gen1Layout::PokedexSeenOff + static_cast<std::size_t>(byteIndex), 4)
             << "\", \"bitIndex\": " << bitInByte
             << ", \"confidence\": \"verified\" }";
        if (dexNo != 151) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteHallOfFameJson(std::ostringstream& json,
                         const SaveBuffer& save,
                         const std::vector<HallOfFameEntry>& hof,
                         const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"entryCount\": " << hof.size() << ",\n";
    json << indent << "  \"sourceRange\": { \"start\": \"" << HexSize(Gen1Layout::HallOfFameOff, 4)
         << "\", \"length\": " << Gen1Layout::HallOfFameLen << " },\n";
    json << indent << "  \"entries\": [\n";
    for (std::size_t i = 0; i < hof.size(); ++i) {
        const HallOfFameEntry& entry = hof[i];
        const std::size_t entryOff = Gen1Layout::HallOfFameOff
            + static_cast<std::size_t>(entry.entryIndex - 1) * Gen1Layout::HallOfFameRecordSize;
        json << indent << "    {\n";
        json << indent << "      \"entryNumber\": " << entry.entryIndex << ",\n";
        json << indent << "      \"sourceRange\": { \"start\": \"" << HexSize(entryOff, 4)
             << "\", \"length\": " << Gen1Layout::HallOfFameRecordSize << " },\n";
        json << indent << "      \"pokemon\": [\n";
        for (std::size_t j = 0; j < entry.team.size(); ++j) {
            const HallOfFamePokemon& mon = entry.team[j];
            const std::size_t monOff = entryOff
                + static_cast<std::size_t>(mon.partyOrder - 1) * Gen1Layout::HallOfFameMonEntrySize;
            json << indent << "        { \"partyOrder\": " << mon.partyOrder
                 << ", \"species\": { \"name\": \"" << JsonEscape(mon.speciesName)
                 << "\", \"internalId\": " << static_cast<int>(mon.speciesId)
                 << ", \"nationalDexNumber\": " << DexNoToInt(Gen1SpeciesLookup::DexfromId(mon.speciesId)) << " }"
                 << ", \"level\": " << static_cast<int>(mon.level)
                 << ", \"nickname\": \"" << JsonEscape(mon.name) << "\""
                 << ", \"nicknameLossless\": \"" << JsonEscape(
                        Gen1TextCodec::DecodeNameLossless(save, monOff + 0x02, 0x0B)) << "\""
                 << ", \"sourceRange\": { \"start\": \"" << HexSize(monOff, 4)
                 << "\", \"length\": " << Gen1Layout::HallOfFameMonEntrySize << " } }";
            if (j + 1 < entry.team.size()) json << ",";
            json << "\n";
        }
        json << indent << "      ]\n";
        json << indent << "    }";
        if (i + 1 < hof.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteNamedFlagsJson(std::ostringstream& json, const FlagSummary& flags, const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"totalFlagsChecked\": " << flags.totalFlagsChecked << ",\n";
    json << indent << "  \"totalFlagsSet\": " << flags.totalFlagsSet << ",\n";
    json << indent << "  \"namedFlagsKnown\": " << flags.namedFlagsKnown << ",\n";
    json << indent << "  \"namedFlagsSet\": " << flags.namedFlagsSet << ",\n";
    json << indent << "  \"beatFlagsSet\": " << flags.beatFlagsSet << ",\n";
    json << indent << "  \"flags\": [\n";
    for (std::size_t i = 0; i < flags.namedFlags.size(); ++i) {
        const auto& flag = flags.namedFlags[i];
        const std::size_t byteOff = Gen1Layout::EventFlagsOff + static_cast<std::size_t>(flag.index / 8);
        const int bit = flag.index % 8;
        json << indent << "    { \"name\": \"" << JsonEscape(flag.name)
             << "\", \"description\": \"" << JsonEscape(EventDescription(flag.name))
             << "\", \"flagIndex\": " << flag.index
             << ", \"byteOffset\": \"" << HexSize(byteOff, 4)
             << "\", \"bitIndex\": " << bit
             << ", \"value\": " << (flag.isSet ? "true" : "false")
             << ", \"category\": \"" << EventCategoryFromName(flag.name)
             << "\", \"persistence\": \"" << EventPersistence(flag.name)
             << "\", \"source\": \"pret/pokered constants/event_constants.asm\", \"confidence\": \"verified\" }";
        if (i + 1 < flags.namedFlags.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteTrainerBattleJson(std::ostringstream& json, const EventCategorySummary& categories, const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"known\": " << categories.trainerFlagsKnown << ",\n";
    json << indent << "  \"complete\": " << categories.trainerFlagsComplete << ",\n";
    json << indent << "  \"defeatedTrainerFlagsSet\": " << categories.defeatedTrainerFlagsSet << ",\n";
    json << indent << "  \"records\": [\n";
    for (std::size_t i = 0; i < categories.trainerFlags.size(); ++i) {
        const auto& flag = categories.trainerFlags[i];
        json << indent << "    { \"name\": \"" << JsonEscape(flag.eventName)
             << "\", \"description\": \"" << JsonEscape(flag.label) << " completed\""
             << ", \"flagIndex\": " << flag.index
             << ", \"completed\": " << (flag.isComplete ? "true" : "false")
             << ", \"location\": \"" << JsonEscape(flag.location)
             << "\", \"trainerNumber\": " << flag.trainerNumber
             << ", \"category\": \"defeated_trainer\", \"persistence\": \"persistent\""
             << ", \"source\": \"pret/pokered\", \"confidence\": \"verified\" }";
        if (i + 1 < categories.trainerFlags.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteStoryFlagRecords(std::ostringstream& json,
                           const std::vector<EventCategorySummary::StoryFlag>& flags,
                           const std::string& indent) {
    json << indent << "[\n";
    for (std::size_t i = 0; i < flags.size(); ++i) {
        const auto& flag = flags[i];
        json << indent << "  { \"name\": \"" << JsonEscape(flag.eventName)
             << "\", \"description\": \"" << JsonEscape(flag.label)
             << "\", \"flagIndex\": " << flag.index
             << ", \"completed\": " << (flag.isComplete ? "true" : "false")
             << ", \"category\": \"" << JsonEscape(flag.category)
             << "\", \"persistence\": \"" << EventPersistence(flag.eventName)
             << "\", \"source\": \"pret/pokered\", \"confidence\": \"verified\" }";
        if (i + 1 < flags.size()) json << ",";
        json << "\n";
    }
    json << indent << "]";
}

void WriteGymConsistencyRecords(std::ostringstream& json,
                                const std::vector<EventCategorySummary::GymConsistency>& gyms,
                                const std::string& indent) {
    json << indent << "[\n";
    for (std::size_t i = 0; i < gyms.size(); ++i) {
        const auto& gym = gyms[i];
        json << indent << "  { \"badgeName\": \"" << JsonEscape(gym.badgeName)
             << "\", \"eventName\": \"" << JsonEscape(gym.eventName)
             << "\", \"badgeOwned\": " << (gym.badgeOwned ? "true" : "false")
             << ", \"leaderEventSet\": " << (gym.leaderEventSet ? "true" : "false")
             << ", \"consistent\": " << (gym.consistent ? "true" : "false")
             << ", \"notes\": \"Badge bit and story flag are reported separately; no automatic story repair is implied.\" }";
        if (i + 1 < gyms.size()) json << ",";
        json << "\n";
    }
    json << indent << "]";
}

void WriteNamedBitStates(std::ostringstream& json,
                         const std::vector<WorldStateSummary::NamedBitState>& states,
                         const std::string& valueName,
                         const std::string& indent) {
    json << indent << "[\n";
    for (std::size_t i = 0; i < states.size(); ++i) {
        const auto& state = states[i];
        json << indent << "  { \"index\": " << state.index
             << ", \"name\": \"" << JsonEscape(state.name)
             << "\", \"location\": \"" << JsonEscape(state.location)
             << "\", \"x\": " << state.x
             << ", \"y\": " << state.y
             << ", \"sprite\": " << state.sprite
             << ", \"defaultState\": \"" << JsonEscape(state.defaultState)
             << "\", \"" << valueName << "\": " << (state.isSet ? "true" : "false")
             << ", \"source\": \"pret/pokered and Junebug metadata\", \"confidence\": \"verified\" }";
        if (i + 1 < states.size()) json << ",";
        json << "\n";
    }
    json << indent << "]";
}

void WriteWorldStateJson(std::ostringstream& json, const WorldStateSummary& world, const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"summary\": { \"missableObjectsSet\": " << world.missableObjectsSet
         << ", \"hiddenItemsCollected\": " << world.hiddenItemsCollected
         << ", \"hiddenCoinsCollected\": " << world.hiddenCoinsCollected
         << ", \"visitedTownsSet\": " << world.visitedTownsSet
         << ", \"currentScriptBytesNonZero\": " << world.currentScriptsNonZero << " },\n";
    json << indent << "  \"storyEvidence\": { \"gotOldRod\": " << (world.gotOldRod ? "true" : "false")
         << ", \"gotGoodRod\": " << (world.gotGoodRod ? "true" : "false")
         << ", \"gotSuperRod\": " << (world.gotSuperRod ? "true" : "false")
         << ", \"satisfiedSaffronGuards\": " << (world.satisfiedSaffronGuards ? "true" : "false")
         << ", \"gotLapras\": " << (world.gotLapras ? "true" : "false")
         << ", \"everHealedPokemon\": " << (world.everHealedPokemon ? "true" : "false")
         << ", \"gotStarter\": " << (world.gotStarter ? "true" : "false")
         << ", \"defeatedLoreleiRoomState\": " << (world.defeatedLorelei ? "true" : "false")
         << " },\n";
    json << indent << "  \"runtimeFields\": [\n";
    for (std::size_t i = 0; i < world.runtimeFields.size(); ++i) {
        const auto& field = world.runtimeFields[i];
        json << indent << "    { \"name\": \"" << JsonEscape(field.name)
             << "\", \"offset\": \"" << JsonEscape(field.offsetHex)
             << "\", \"value\": \"" << JsonEscape(field.value)
             << "\", \"source\": \"" << JsonEscape(field.source)
             << "\", \"persistence\": \"runtime\", \"confidence\": \"strongly_supported\" }";
        if (i + 1 < world.runtimeFields.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

void WriteCurrentScriptsJson(std::ostringstream& json, const WorldStateSummary& world, const std::string& indent) {
    json << indent << "{\n";
    json << indent << "  \"count\": " << world.currentScriptsChecked << ",\n";
    json << indent << "  \"nonZeroCount\": " << world.currentScriptsNonZero << ",\n";
    json << indent << "  \"scripts\": [\n";
    for (std::size_t i = 0; i < world.currentScripts.size(); ++i) {
        const auto& script = world.currentScripts[i];
        json << indent << "    { \"index\": " << script.index
             << ", \"mapOrScriptName\": \"" << JsonEscape(script.name)
             << "\", \"relativeOffset\": " << script.relativeOffset
             << ", \"absoluteOffset\": \"" << HexSize(Gen1Layout::CurrentScriptsOff + static_cast<std::size_t>(script.relativeOffset), 4)
             << "\", \"size\": " << script.size
             << ", \"rawValue\": " << script.value
             << ", \"interpretationStatus\": \"unverified_script_semantics\", \"confidence\": \"strongly_supported\" }";
        if (i + 1 < world.currentScripts.size()) json << ",";
        json << "\n";
    }
    json << indent << "  ]\n";
    json << indent << "}";
}

std::string BuildCoverageJson(const RedCoverageSummary& coverage) {
    std::ostringstream json;
    json << "  \"coverage\": {\n";
    json << "    \"summary\": {\n";
    json << "      \"totalStandardSramBytes\": " << coverage.totalStandardSramBytes << ",\n";
    json << "      \"decodedBytes\": " << coverage.decodedBytes << ",\n";
    json << "      \"partiallyDecodedBytes\": " << coverage.partiallyDecodedBytes << ",\n";
    json << "      \"knownRawBytes\": " << coverage.knownRawBytes << ",\n";
    json << "      \"runtimeBytes\": " << coverage.runtimeBytes << ",\n";
    json << "      \"unknownBytes\": " << coverage.unknownBytes << ",\n";
    json << "      \"rawPreservedBytes\": " << coverage.rawPreservedBytes << ",\n";
    json << "      \"uncoveredBytes\": " << coverage.uncoveredBytes << ",\n";
    json << "      \"overlappingPrimaryBytes\": " << coverage.overlappingPrimaryBytes << "\n";
    json << "    },\n";
    json << "    \"primaryRanges\": [\n";
    for (std::size_t i = 0; i < coverage.ranges.size(); ++i) {
        const RedCoverageRange& r = coverage.ranges[i];
        json << "      {\n";
        json << "        \"start\": \"" << HexSize(r.start, 4) << "\",\n";
        json << "        \"endInclusive\": \"" << HexSize(r.endInclusive, 4) << "\",\n";
        json << "        \"length\": " << (r.endInclusive - r.start + 1) << ",\n";
        json << "        \"name\": \"" << JsonEscape(r.name) << "\",\n";
        json << "        \"classification\": \"" << JsonEscape(r.classification) << "\",\n";
        json << "        \"decoded\": " << (r.decoded ? "true" : "false") << ",\n";
        json << "        \"reconstructionPolicy\": \"" << JsonEscape(r.reconstructionPolicy) << "\",\n";
        json << "        \"sourceReferences\": [],\n";
        json << "        \"notes\": \"" << JsonEscape(r.notes) << "\"\n";
        json << "      }";
        if (i + 1 < coverage.ranges.size()) json << ",";
        json << "\n";
    }
    json << "    ]\n";
    json << "  },\n";
    return json.str();
}

std::string BuildDecodedJson(const SaveBuffer& save, const ReadOnlyData& reader, bool includeDecodedSummary) {
    std::ostringstream json;
    json << "  \"decoded\": {\n";
    json << "    \"status\": \"expanded_current_save_genie_model\",\n";
    json << "    \"confidence\": \"partially_decoded_but_losslessly_preserved\",\n";
    json << "    \"reconstructionRole\": \"informational_only_in_schema_0_1_0\"";
    if (!includeDecodedSummary) {
        json << "\n";
        json << "  },\n";
        return json.str();
    }

    try {
        const TrainerSummary trainer = reader.GetTrainerSummary();
        const PlayerStateSummary player = reader.GetPlayerStateSummary();
        const PokedexSummary pokedex = reader.GetPokedexSummary(true);
        const BagSummary bag = reader.GetBagSummary(true);
        const BagSummary pcItems = reader.GetPCItemBoxSummary(true);
        const PokemonBoxesExport boxes = reader.GetAllBoxesExport();
        const PokemonBox& party = boxes.boxes.front();
        const PokemonBox& currentBoxCache = boxes.currentBoxCache;
        const FlagSummary events = reader.GetEventFlagSummary();
        const EventCategorySummary categories = reader.GetEventCategorySummary();
        const WorldStateSummary world = reader.GetWorldStateSummary();
        const DaycareSummary daycare = reader.GetDaycareSummary();
        const std::vector<HallOfFameEntry> hof = reader.GetHallOfFame();

        static constexpr std::size_t kCurrentBoxIndexOff = 0x284C;
        const u8 rawCurrentBox = save.ReadU8(kCurrentBoxIndexOff);
        const int selectedBoxZeroBased = static_cast<int>(rawCurrentBox & 0x7F);
        const bool selectedBoxValid = selectedBoxZeroBased >= 0 && selectedBoxZeroBased < 12;
        const int selectedBoxOneBased = selectedBoxValid ? selectedBoxZeroBased + 1 : -1;
        std::size_t cacheDiffs = 0;
        std::string cacheSync = "unresolved";
        if (selectedBoxValid) {
            const FileManipulation::Bytes permanent =
                save.Slice(Gen1Layout::BoxBaseOffsetByIndex1to12(selectedBoxOneBased), Gen1Layout::BoxBlockSize);
            const FileManipulation::Bytes cache =
                save.Slice(Gen1Layout::CurrentBoxCacheOff, Gen1Layout::CurrentBoxCacheLen);
            cacheDiffs = RedCountByteDifferences(permanent, cache);
            cacheSync = (cacheDiffs == 0) ? "matching" : "different";
        }

        json << ",\n";
        json << "    \"trainer\": {\n";
        json << "      \"name\": { \"value\": \"" << JsonEscape(trainer.trainerName)
             << "\", \"losslessValue\": \"" << JsonEscape(Gen1TextCodec::DecodeNameLossless(
                    save, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen))
             << "\", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen)
             << "\", " << FieldMeta(Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen, "gen1_text") << " },\n";
        json << "      \"trainerId\": { \"value\": " << trainer.trainerId
             << ", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::TrainerIdOff, 2)
             << "\", " << FieldMeta(Gen1Layout::TrainerIdOff, 2, "big_endian_u16") << " }\n";
        json << "    },\n";
        json << "    \"rival\": {\n";
        json << "      \"name\": { \"value\": \"" << JsonEscape(trainer.rivalName)
             << "\", \"losslessValue\": \"" << JsonEscape(Gen1TextCodec::DecodeNameLossless(
                    save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen))
             << "\", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen)
             << "\", " << FieldMeta(Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen, "gen1_text") << " }\n";
        json << "    },\n";
        json << "    \"options\": {\n";
        json << "      \"optionsByte\": { \"value\": " << static_cast<int>(player.optionsByte)
             << ", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::OptionsOff, 1)
             << "\", " << FieldMeta(Gen1Layout::OptionsOff, 1, "bitfield", "strongly_supported") << " },\n";
        json << "      \"letterDelayByte\": { \"value\": " << static_cast<int>(player.letterDelayByte)
             << ", \"offset\": \"" << HexSize(Gen1Layout::LetterDelayOff, 4) << "\" },\n";
        json << "      \"contrast\": { \"value\": " << static_cast<int>(player.contrast)
             << ", \"offset\": \"" << HexSize(Gen1Layout::ContrastOff, 4) << "\" }\n";
        json << "    },\n";
        json << "    \"playtime\": { \"hours\": " << static_cast<int>(trainer.playHours)
             << ", \"minutes\": " << static_cast<int>(trainer.playMinutes)
             << ", \"seconds\": " << static_cast<int>(trainer.playSeconds)
             << ", \"sourceOffsets\": { \"hours\": \"" << HexSize(Gen1Layout::PlayTimeHoursOff, 4)
             << "\", \"minutes\": \"" << HexSize(Gen1Layout::PlayTimeMinutesOff, 4)
             << "\", \"seconds\": \"" << HexSize(Gen1Layout::PlayTimeSecondsOff, 4) << "\" } },\n";
        json << "    \"moneyAndCoins\": {\n";
        json << "      \"money\": { \"value\": " << trainer.money
             << ", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::MoneyOff, Gen1Layout::MoneyLen)
             << "\", " << FieldMeta(Gen1Layout::MoneyOff, Gen1Layout::MoneyLen, "packed_bcd") << " },\n";
        json << "      \"coins\": { \"value\": " << trainer.coins
             << ", \"rawHex\": \"" << RawHexAt(save, Gen1Layout::CoinsOff, Gen1Layout::CoinsLen)
             << "\", " << FieldMeta(Gen1Layout::CoinsOff, Gen1Layout::CoinsLen, "packed_bcd") << " }\n";
        json << "    },\n";
        json << "    \"badges\": {\n";
        json << "      \"rawBitfield\": \"" << HexSize(trainer.badges, 2) << "\",\n";
        WriteBadgeArray(json, trainer.badges);
        json << "\n";
        json << "    },\n";
        json << "    \"location\": {\n";
        json << "      \"map\": { \"id\": " << static_cast<int>(trainer.mapId)
             << ", \"hex\": \"" << Gen1MapLookup::MapIDHex[static_cast<int>(trainer.mapId)]
             << "\", \"name\": \"" << JsonEscape(Gen1MapLookup::NameFromId(trainer.mapId))
             << "\", \"offset\": \"" << HexSize(Gen1Layout::MapIdOff, 4) << "\" },\n";
        json << "      \"x\": { \"value\": " << static_cast<int>(trainer.x)
             << ", \"offset\": \"" << HexSize(Gen1Layout::XCoordOff, 4) << "\" },\n";
        json << "      \"y\": { \"value\": " << static_cast<int>(trainer.y)
             << ", \"offset\": \"" << HexSize(Gen1Layout::YCoordOff, 4) << "\" },\n";
        json << "      \"previousMap\": { \"id\": " << static_cast<int>(save.ReadU8(Gen1Layout::LastMapOff))
             << ", \"name\": \"" << JsonEscape(Gen1MapLookup::NameFromId(save.ReadU8(Gen1Layout::LastMapOff)))
             << "\", \"offset\": \"" << HexSize(Gen1Layout::LastMapOff, 4) << "\" }\n";
        json << "    },\n";
        json << "    \"runtimeState\": {\n";
        json << "      \"movementMode\": \"" << JsonEscape(player.MovementModeName()) << "\",\n";
        json << "      \"xBlockCoord\": " << static_cast<int>(player.xBlockCoord) << ",\n";
        json << "      \"yBlockCoord\": " << static_cast<int>(player.yBlockCoord) << ",\n";
        json << "      \"playerMoveDirection\": \"" << JsonEscape(player.DirectionName(player.playerMoveDir)) << "\",\n";
        json << "      \"playerCurrentDirection\": \"" << JsonEscape(player.DirectionName(player.playerCurDir)) << "\",\n";
        json << "      \"safari\": { \"gameOver\": " << (player.safariGameOver ? "true" : "false")
             << ", \"ballCount\": " << static_cast<int>(player.safariBallCount)
             << ", \"steps\": " << player.safariSteps << " },\n";
        json << "      \"flags\": { \"strengthOutsideBattle\": " << (player.strengthOutsideBattle ? "true" : "false")
             << ", \"surfingAllowed\": " << (player.surfingAllowed ? "true" : "false")
             << ", \"flyOutOfBattle\": " << (player.flyOutOfBattle ? "true" : "false")
             << ", \"isBattle\": " << (player.isBattle ? "true" : "false")
             << ", \"isTrainerBattle\": " << (player.isTrainerBattle ? "true" : "false")
             << ", \"countPlaytime\": " << (player.countPlaytime ? "true" : "false") << " }\n";
        json << "    },\n";
        json << "    \"pokedex\": ";
        WritePokedexJson(json, pokedex, "    ");
        json << ",\n";
        json << "    \"inventory\": {\n";
        json << "      \"bag\": ";
        WriteBagSummaryJson(json, bag, Gen1Layout::BagItemsCountOff, Gen1Layout::BagItemsPairsOff, "Bag", "      ");
        json << ",\n";
        json << "      \"pcItemStorage\": ";
        WriteBagSummaryJson(json, pcItems, Gen1Layout::PCItemBoxCountOff, Gen1Layout::PCItemBoxPairsOff, "PC Item Storage", "      ");
        json << ",\n";
        json << "      \"keyItemsAndMachines\": { \"status\": \"derived_from_item_ids\", \"notes\": \"Item categories are readable by item name; dedicated key-item/HM/TM grouping is planned for schema 0.2.x.\" }\n";
        json << "    },\n";
        json << "    \"party\": ";
        WritePokemonBoxCanonical(json, save, party, Gen1Layout::PartyBase, true, "    ");
        json << ",\n";
        json << "    \"pcStorage\": {\n";
        json << "      \"boxCount\": 12,\n";
        json << "      \"boxes\": [\n";
        for (int i = 1; i <= 12; ++i) {
            const PokemonBox& pcBox = boxes.boxes[static_cast<std::size_t>(i)];
            WritePokemonBoxCanonical(json, save, pcBox, Gen1Layout::BoxBaseOffsetByIndex1to12(i), false, "        ");
            if (i != 12) json << ",";
            json << "\n";
        }
        json << "      ]\n";
        json << "    },\n";
        json << "    \"currentBoxCache\": {\n";
        json << "      \"rawCurrentBoxByte\": \"" << HexSize(rawCurrentBox, 2) << "\",\n";
        json << "      \"currentBoxByteOffset\": \"" << HexSize(kCurrentBoxIndexOff, 4) << "\",\n";
        json << "      \"selectedBoxNumber\": ";
        if (selectedBoxValid) json << selectedBoxOneBased << ",\n";
        else json << "null,\n";
        json << "      \"hasChangedBoxesBefore\": "
             << ((rawCurrentBox & 0x80) ? "true" : "false") << ",\n";
        json << "      \"boxChangedFlag\": "
             << ((rawCurrentBox & 0x80) ? "true" : "false")
             << ",\n"; // backward-compatible draft-schema alias
        json << "      \"correspondingPermanentBox\": ";
        if (selectedBoxValid) json << selectedBoxOneBased << ",\n";
        else json << "null,\n";
        json << "      \"synchronizationStatus\": \"" << cacheSync << "\",\n";
        json << "      \"byteDifferencesAgainstPermanentBox\": " << cacheDiffs << ",\n";
        json << "      \"cache\": ";
        WritePokemonBoxCanonical(json, save, currentBoxCache, Gen1Layout::CurrentBoxCacheOff, false, "      ");
        json << "\n";
        json << "    },\n";
        json << "    \"daycare\": {\n";
        json << "      \"inUse\": " << (daycare.inUse ? "true" : "false") << ",\n";
        json << "      \"inUseOffset\": \"" << HexSize(Gen1Layout::DaycareInUseOff, 4) << "\",\n";
        json << "      \"pokemon\": ";
        if (daycare.inUse) {
            WritePokemonMonJson(json, save, daycare.pokemon, Gen1Layout::DaycareBoxMonOff, Gen1Layout::BoxStructSize,
                                Gen1Layout::DaycareBoxMonOff, Gen1Layout::DaycareOTNameOff,
                                Gen1Layout::DaycareNicknameOff, false, "      ");
            json << "\n";
        } else {
            json << "null\n";
        }
        json << "    },\n";
        json << "    \"hallOfFame\": ";
        WriteHallOfFameJson(json, save, hof, "    ");
        json << ",\n";
        json << "    \"events\": ";
        WriteNamedFlagsJson(json, events, "    ");
        json << ",\n";
        json << "    \"trainerBattles\": ";
        WriteTrainerBattleJson(json, categories, "    ");
        json << ",\n";
        json << "    \"staticBattles\": {\n";
        json << "      \"known\": " << categories.staticEncounterFlagsKnown << ",\n";
        json << "      \"complete\": " << categories.staticEncounterFlagsComplete << ",\n";
        json << "      \"legendaryFlagsSet\": " << categories.legendaryFlagsSet << ",\n";
        json << "      \"records\": ";
        WriteStoryFlagRecords(json, categories.staticEncounterFlags, "      ");
        json << "\n";
        json << "    },\n";
        json << "    \"storyProgress\": {\n";
        json << "      \"known\": " << categories.storyFlagsKnown << ",\n";
        json << "      \"complete\": " << categories.storyFlagsComplete << ",\n";
        json << "      \"majorStoryMilestonesSet\": " << categories.majorStoryMilestonesSet << ",\n";
        json << "      \"gymStoryMismatchCount\": " << categories.gymStoryMismatchCount << ",\n";
        json << "      \"eliteFourHistoricalCompletion\": { \"inferredFromHallOfFame\": "
             << (!hof.empty() ? "true" : "false")
             << ", \"rawRoomFlagsRemainSeparate\": true },\n";
        json << "      \"storyFlags\": ";
        WriteStoryFlagRecords(json, categories.storyFlags, "      ");
        json << ",\n";
        json << "      \"gymConsistency\": ";
        WriteGymConsistencyRecords(json, categories.gymConsistency, "      ");
        json << "\n";
        json << "    },\n";
        json << "    \"scripts\": ";
        WriteCurrentScriptsJson(json, world, "    ");
        json << ",\n";
        json << "    \"missableObjects\": ";
        WriteNamedBitStates(json, world.missableObjects, "toggledOff", "    ");
        json << ",\n";
        json << "    \"hiddenItems\": ";
        WriteNamedBitStates(json, world.hiddenItems, "collected", "    ");
        json << ",\n";
        json << "    \"hiddenCoins\": ";
        WriteNamedBitStates(json, world.hiddenCoins, "collected", "    ");
        json << ",\n";
        json << "    \"visitedTowns\": ";
        WriteNamedBitStates(json, world.visitedTowns, "visited", "    ");
        json << ",\n";
        json << "    \"worldState\": ";
        WriteWorldStateJson(json, world, "    ");
        json << ",\n";
        json << "    \"checksums\": {\n";
        json << "      \"mainValid\": " << (Gen1Checksum::ValidateMain(save) ? "true" : "false") << ",\n";
        json << "      \"bank2AllValid\": " << (Gen1Checksum::ValidateBankAll(save, 2) ? "true" : "false") << ",\n";
        json << "      \"bank3AllValid\": " << (Gen1Checksum::ValidateBankAll(save, 3) ? "true" : "false") << "\n";
        json << "    },\n";
        json << "    \"semanticConsistency\": {\n";
        json << "      \"partyCountMatchesArray\": " << (party.pokemonCount == static_cast<int>(party.pokemon.size()) ? "true" : "false") << ",\n";
        json << "      \"pokedexOwnedCountMatchesList\": " << (pokedex.ownedCount == static_cast<int>(pokedex.ownedDexNos.size()) ? "true" : "false") << ",\n";
        json << "      \"pokedexSeenCountMatchesList\": " << (pokedex.seenCount == static_cast<int>(pokedex.seenDexNos.size()) ? "true" : "false") << ",\n";
        json << "      \"bagCountMatchesArray\": " << (bag.itemCount == static_cast<int>(bag.items.size()) ? "true" : "false") << ",\n";
        json << "      \"pcItemCountMatchesArray\": " << (pcItems.itemCount == static_cast<int>(pcItems.items.size()) ? "true" : "false") << ",\n";
        json << "      \"hallOfFameEntryCountMatchesArray\": true,\n";
        json << "      \"eventNamedSetCountMatchesArray\": " << (events.namedFlagsSet == static_cast<int>(events.namedSetFlags.size()) ? "true" : "false") << ",\n";
        json << "      \"semanticValidationBlocksRawReconstruction\": false\n";
        json << "    }\n";
    } catch (const std::exception& e) {
        json << ",\n";
        json << "    \"decodeError\": \"" << JsonEscape(e.what()) << "\"\n";
    }

    json << "  },\n";
    return json.str();
}

std::string BuildConversionModelJson(const ReadOnlyData& reader) {
    std::ostringstream json;
    const TrainerSummary trainer = reader.GetTrainerSummary();
    const PokedexSummary pokedex = reader.GetPokedexSummary(true);
    const BagSummary bag = reader.GetBagSummary(true);
    const BagSummary pcItems = reader.GetPCItemBoxSummary(true);
    const PokemonBoxesExport boxes = reader.GetAllBoxesExport();
    const PokemonBox party = boxes.boxes.empty() ? PokemonBox{} : boxes.boxes.front();
    const DaycareSummary daycare = reader.GetDaycareSummary();
    const EventCategorySummary categories = reader.GetEventCategorySummary();
    const WorldStateSummary world = reader.GetWorldStateSummary();
    const std::vector<HallOfFameEntry> hof = reader.GetHallOfFame();

    auto countItemsByStatus = [](const BagSummary& summary, const std::string& status) {
        int count = 0;
        for (const BagItem& item : summary.items) {
            if (ItemConversionStatus(item.itemName) == status) ++count;
        }
        return count;
    };

    int storagePokemonCount = 0;
    for (std::size_t i = 1; i < boxes.boxes.size(); ++i) {
        storagePokemonCount += boxes.boxes[i].pokemonCount;
    }

    json << "  \"conversionModel\": {\n";
    json << "    \"modelVersion\": \"0.1.0-draft\",\n";
    json << "    \"purpose\": \"layout_independent_red_to_firered_source_model\",\n";
    json << "    \"pipeline\": [\"red_json\", \"shared_conversion_model\", \"fred_json\", \"firered_save_writer\"],\n";
    json << "    \"authority\": { \"archival\": \"physicalImage\", \"semantic\": \"decoded\", \"reconstruction\": \"physicalImage_only_for_no_edit_round_trip\" },\n";
    json << "    \"identity\": {\n";
    json << "      \"sourcePath\": \"$.decoded.trainer\",\n";
    json << "      \"playerName\": \"" << JsonEscape(trainer.trainerName) << "\",\n";
    json << "      \"trainerId\": " << trainer.trainerId << ",\n";
    json << "      \"classification\": \"direct_transfer\",\n";
    json << "      \"targetMapping\": \"fire_red_trainer_identity\",\n";
    json << "      \"genderPolicy\": { \"classification\": \"initialize_to_safe_default\", \"default\": \"male\", \"reason\": \"Pokemon Red stores no player gender; future UI may override before conversion\" },\n";
    json << "      \"confidence\": \"verified\"\n";
    json << "    },\n";
    json << "    \"trainer\": { \"sourcePath\": \"$.decoded.trainer\", \"classification\": \"direct_transfer\", \"textPolicy\": \"gen1_text_to_fire_red_text_normalized_utf8_then_encoded_by_target_writer\" },\n";
    json << "    \"playtime\": { \"sourcePath\": \"$.decoded.playtime\", \"classification\": \"direct_transfer\", \"hours\": " << static_cast<int>(trainer.playHours)
         << ", \"minutes\": " << static_cast<int>(trainer.playMinutes)
         << ", \"seconds\": " << static_cast<int>(trainer.playSeconds) << " },\n";
    json << "    \"currency\": { \"sourcePath\": \"$.decoded.moneyAndCoins\", \"classification\": \"direct_transfer\", \"money\": " << trainer.money
         << ", \"coins\": " << trainer.coins << ", \"targetPolicy\": \"respect_fire_red_caps_during_target_write\" },\n";
    json << "    \"badges\": { \"sourcePath\": \"$.decoded.badges.badges\", \"classification\": \"direct_transfer\", \"badgeByte\": \"" << HexSize(trainer.badges, 2)
         << "\", \"storyFlagsPolicy\": \"badge_bits_transfer_directly_but_related_story_flags_are_semantic_translation\" },\n";
    json << "    \"pokedex\": { \"sourcePath\": \"$.decoded.pokedex\", \"classification\": \"direct_transfer\", \"ownedCount\": " << pokedex.ownedCount
         << ", \"seenCount\": " << pokedex.seenCount << ", \"targetMapping\": \"national_dex_species_identity\" },\n";
    json << "    \"party\": { \"sourcePath\": \"$.decoded.party\", \"classification\": \"direct_transfer\", \"count\": " << party.pokemonCount
         << ", \"pokemonPolicy\": \"use_per_pokemon_conversion_records\" },\n";
    json << "    \"storage\": { \"sourcePath\": \"$.decoded.pcStorage\", \"classification\": \"direct_transfer\", \"boxCount\": 12, \"pokemonCount\": " << storagePokemonCount
         << ", \"boxCapacityPolicy\": \"fire_red_capacity_is_larger; preserve_order_then_fill_empty_slots\" },\n";
    json << "    \"daycare\": { \"sourcePath\": \"$.decoded.daycare\", \"classification\": \"semantic_translation\", \"inUse\": " << (daycare.inUse ? "true" : "false")
         << ", \"targetPolicy\": \"transfer_if_fire_red_daycare_state_is_available_else_report_warning\" },\n";
    json << "    \"inventory\": {\n";
    json << "      \"sourcePath\": \"$.decoded.inventory\",\n";
    json << "      \"classification\": \"mixed\",\n";
    json << "      \"bagItems\": " << bag.items.size() << ",\n";
    json << "      \"pcItems\": " << pcItems.items.size() << ",\n";
    json << "      \"directByName\": " << (countItemsByStatus(bag, "direct_by_name") + countItemsByStatus(pcItems, "direct_by_name")) << ",\n";
    json << "      \"semanticTranslation\": " << (countItemsByStatus(bag, "semantic_translation") + countItemsByStatus(pcItems, "semantic_translation")) << ",\n";
    json << "      \"policyRequired\": " << (countItemsByStatus(bag, "policy_required") + countItemsByStatus(pcItems, "policy_required")) << ",\n";
    json << "      \"unsupported\": " << (countItemsByStatus(bag, "unsupported") + countItemsByStatus(pcItems, "unsupported")) << "\n";
    json << "    },\n";
    json << "    \"storyProgress\": { \"sourcePath\": \"$.decoded.storyProgress\", \"classification\": \"semantic_translation\", \"majorStoryMilestonesSet\": " << categories.majorStoryMilestonesSet
         << ", \"gymStoryMismatchCount\": " << categories.gymStoryMismatchCount
         << ", \"eliteFourHistoricalEvidence\": \"" << (!hof.empty() ? "hall_of_fame_present" : "not_found") << "\" },\n";
    json << "    \"staticEncounters\": { \"sourcePath\": \"$.decoded.staticBattles\", \"classification\": \"semantic_translation\", \"complete\": " << categories.staticEncounterFlagsComplete
         << ", \"legendaryFlagsSet\": " << categories.legendaryFlagsSet << " },\n";
    json << "    \"hallOfFame\": { \"sourcePath\": \"$.decoded.hallOfFame\", \"classification\": \"semantic_translation\", \"entryCount\": " << hof.size()
         << ", \"policy\": \"use_as_evidence_for_historical_league_completion; direct_firered_hall_of_fame_reconstruction_requires_p2_research\" },\n";
    json << "    \"locationPolicy\": { \"sourcePath\": \"$.decoded.location\", \"classification\": \"semantic_translation\", \"sourceMapId\": " << static_cast<int>(trainer.mapId)
         << ", \"sourceMapName\": \"" << JsonEscape(Gen1MapLookup::NameFromId(trainer.mapId))
         << "\", \"targetPolicy\": \"map_to_known_good_firered_spawn_by_semantic_location_not_exact_tile_when_needed\", \"flyDestinationsSet\": " << world.visitedTownsSet << " },\n";
    json << "    \"conversionPolicies\": {\n";
    json << "      \"species\": { \"basis\": \"national_dex_number\", \"numericIdsAreNotSharedAcrossGenerations\": true },\n";
    json << "      \"moves\": { \"basis\": \"move_name_and_semantic_move_identity\", \"numericIdsAreNotAssumedEqual\": true, \"unsupportedMoves\": \"report_and_reject_or_policy_replace\" },\n";
    json << "      \"items\": { \"basis\": \"semantic_item_identity\", \"tmHmPolicy\": \"map_by_taught_move\", \"keyItemPolicy\": \"prefer_story_state_when_fire_red_inventory_conflicts\" },\n";
    json << "      \"text\": { \"sourceEncoding\": \"gen1_text\", \"normalizedForm\": \"utf8_ascii_subset\", \"unsupportedCharacters\": \"report_and_require_policy\" },\n";
    json << "      \"status\": { \"basis\": \"semantic_status_condition\", \"partyOnly\": true, \"boxedStatusPolicy\": \"not_stored_in_gen_i_box_structure\" },\n";
    json << "      \"dvsToIvs\": { \"draftPolicy\": \"iv = dv * 2 + 1\", \"specialMapsTo\": [\"specialAttack\", \"specialDefense\"], \"confidence\": \"policy_draft\" },\n";
    json << "      \"statExperienceToEvs\": { \"draftPolicy\": \"floor(statExp / 256), split Gen I Special to SpA and SpD, scale to Gen III total cap 510\", \"classification\": \"approximate_deterministically\" },\n";
    json << "      \"generatedPokemonFields\": { \"pidNatureGenderAbility\": \"derive_deterministically_from_per_pokemon_seed_after_gen_iii_codec_exists\", \"heldItems\": \"none_safe_default\", \"ribbons\": \"none_safe_default\" }\n";
    json << "    },\n";
    json << "    \"classificationTable\": {\n";
    json << "      \"directTransfer\": [\"trainer_identity\", \"money\", \"coins\", \"playtime\", \"badges\", \"pokedex_species_states\", \"party_pokemon\", \"pc_pokemon\"],\n";
    json << "      \"semanticTranslation\": [\"story_milestones\", \"trainer_battles\", \"rival_battles\", \"static_encounters\", \"legendary_state\", \"fossil_choice\", \"gift_pokemon\", \"key_items\", \"location\", \"box_selection\", \"daycare\"],\n";
    json << "      \"redOnlyPreservation\": [\"physicalImage\", \"unknownData\", \"padding\", \"temporary_runtime_flags\", \"obsolete_scripts\", \"map_local_scratch_values\"],\n";
    json << "      \"unsupportedOrPolicyRequired\": [\"glitch_species\", \"glitch_moves\", \"unsupported_text\", \"incompatible_items\", \"impossible_stats\", \"ambiguous_story_flags\", \"fire_red_exclusive_state\"]\n";
    json << "    },\n";
    json << "    \"warnings\": [\n";
    json << "      \"This is a Red-side conversion-ready draft, not a completed Red-to-FireRed converter.\",\n";
    json << "      \"FireRed .fred.json, FireRed checksums, Pokemon encryption, and target save writing are not implemented in this milestone.\",\n";
    json << "      \"Draft Pokemon IV/EV/generated-field policies are deterministic but require review before declaring converted saves reliable.\"\n";
    json << "    ],\n";
    json << "    \"provenance\": [\n";
    json << "      { \"source\": \"local Save Genie parser\", \"role\": \"implemented Red decoded data\" },\n";
    json << "      { \"source\": \"pret/pokered\", \"role\": \"Gen I symbols, flags, events, maps, and constants\" },\n";
    json << "      { \"source\": \"Junebug pokered-save-editor repositories\", \"role\": \"research cross-check for save fields and editor coverage\" },\n";
    json << "      { \"source\": \"future pret/pokefirered research\", \"role\": \"FireRed target mappings and writer validation\" }\n";
    json << "    ]\n";
    json << "  },\n";
    return json.str();
}

std::string BuildUnknownDataJson(const RedCoverageSummary& coverage) {
    std::ostringstream json;
    json << "  \"unknownData\": {\n";
    json << "    \"policy\": \"preserved_by_physical_image\",\n";
    json << "    \"ranges\": [\n";
    bool first = true;
    for (const RedCoverageRange& r : coverage.ranges) {
        if (r.classification != "unknown" && r.classification != "raw_preserved") continue;
        if (!first) json << ",\n";
        first = false;
        json << "      { \"start\": \"" << HexSize(r.start, 4)
             << "\", \"endInclusive\": \"" << HexSize(r.endInclusive, 4)
             << "\", \"classification\": \"" << r.classification << "\" }";
    }
    if (!first) json << "\n";
    json << "    ]\n";
    json << "  },\n";
    return json.str();
}

std::string BuildMasterJson(const std::string& inputPath,
                            const SaveBuffer& save,
                            const ReadOnlyData& reader,
                            const RedMasterJsonOptions& options,
                            const RedCoverageSummary& coverage,
                            std::vector<std::string>* warnings) {
    const FileManipulation::Bytes& allBytes = save.BytesView();
    const std::size_t totalLength = allBytes.size();
    if (totalLength < Gen1Layout::ExpectedSize) {
        throw std::runtime_error("Cannot export .red.json: save is shorter than the required 0x8000-byte Gen I SRAM region.");
    }

    const FileManipulation::Bytes standard = SliceBytes(allBytes, 0, Gen1Layout::ExpectedSize);
    const std::size_t trailingLength = totalLength - Gen1Layout::ExpectedSize;
    const FileManipulation::Bytes trailing = SliceBytes(allBytes, Gen1Layout::ExpectedSize, trailingLength);

    if (trailingLength > 0 && warnings) {
        warnings->push_back("This file contains " + HexSize(trailingLength) +
                            " trailing bytes beyond the standard 0x8000-byte Gen I SRAM region. "
                            "The standard save region will be decoded, and trailing bytes are preserved unchanged.");
    }

    const std::filesystem::path p(inputPath);
    const std::string fileName = p.filename().string();
    const std::string extension = p.extension().string();
    const std::string wholeHash = RedMasterJsonSha256Hex(allBytes);
    const std::string standardHash = RedMasterJsonSha256Hex(standard);
    const std::string trailingHash = trailing.empty() ? std::string() : RedMasterJsonSha256Hex(trailing);

    std::ostringstream json;
    json << "{\n";
    json << "  \"schema\": {\n";
    json << "    \"format\": \"" << kFormat << "\",\n";
    json << "    \"schemaVersion\": \"" << kSchemaVersion << "\",\n";
    json << "    \"game\": \"Pokemon Red\",\n";
    json << "    \"generation\": 1,\n";
    json << "    \"regionAssumption\": \"USA-Europe\",\n";
    json << "    \"endiannessConvention\": \"explicit-per-field\",\n";
    json << "    \"canonicalExtension\": \".red.json\",\n";
    json << "    \"lossless\": true,\n";
    json << "    \"stability\": \"draft\"\n";
    json << "  },\n";

    json << "  \"source\": {\n";
    json << "    \"fileName\": \"" << JsonEscape(fileName) << "\",\n";
    json << "    \"originalExtension\": \"" << JsonEscape(extension) << "\",\n";
    json << "    \"fileSize\": { \"decimal\": " << totalLength << ", \"hex\": \"" << HexSize(totalLength) << "\" },\n";
    json << "    \"standardSramSize\": { \"decimal\": " << Gen1Layout::ExpectedSize
         << ", \"hex\": \"" << HexSize(Gen1Layout::ExpectedSize) << "\" },\n";
    json << "    \"trailingByteCount\": " << trailingLength << ",\n";
    json << "    \"hashes\": {\n";
    json << "      \"wholeFileSha256\": \"" << wholeHash << "\",\n";
    json << "      \"standardSramSha256\": \"" << standardHash << "\",\n";
    json << "      \"trailingDataSha256\": ";
    if (trailing.empty()) json << "null\n";
    else json << "\"" << trailingHash << "\"\n";
    json << "    },\n";
    json << "    \"generatedAtUtc\": ";
    if (options.includeGeneratedAtUtc) json << "\"" << UtcTimestamp() << "\",\n";
    else json << "null,\n";
    json << "    \"exporter\": \"Pkmn Red Save Genie\",\n";
    json << "    \"warnings\": [";
    if (warnings && !warnings->empty()) {
        json << "\n";
        for (std::size_t i = 0; i < warnings->size(); ++i) {
            json << "      \"" << JsonEscape((*warnings)[i]) << "\"";
            if (i + 1 < warnings->size()) json << ",";
            json << "\n";
        }
        json << "    ";
    }
    json << "]\n";
    json << "  },\n";

    json << "  \"integrity\": {\n";
    json << "    \"mainChecksum\": {\n";
    json << "      \"storedOffset\": \"" << HexSize(Gen1Layout::MainChecksumOff, 4) << "\",\n";
    json << "      \"coveredRange\": { \"start\": \"" << HexSize(Gen1Layout::MainChecksumStart, 4)
         << "\", \"endInclusive\": \"" << HexSize(Gen1Layout::MainChecksumEnd, 4) << "\" },\n";
    json << "      \"storedValue\": \"" << HexSize(save.ReadU8(Gen1Layout::MainChecksumOff), 2) << "\",\n";
    json << "      \"calculatedValue\": \"" << HexSize(Gen1Checksum::ComputeMain(save), 2) << "\",\n";
    json << "      \"valid\": " << (Gen1Checksum::ValidateMain(save) ? "true" : "false") << ",\n";
    json << "      \"algorithm\": \"invert-low-byte-of-sum\"\n";
    json << "    },\n";
    json << "    \"bank2AllChecksumValid\": " << (Gen1Checksum::ValidateBankAll(save, 2) ? "true" : "false") << ",\n";
    json << "    \"bank3AllChecksumValid\": " << (Gen1Checksum::ValidateBankAll(save, 3) ? "true" : "false") << "\n";
    json << "  },\n";

    json << "  \"physicalImage\": {\n";
    json << "    \"encoding\": \"" << kHexEncoding << "\",\n";
    json << "    \"standardSramHex\": \"" << RedBytesToCanonicalHex(standard) << "\",\n";
    json << "    \"trailingDataHex\": \"" << RedBytesToCanonicalHex(trailing) << "\",\n";
    json << "    \"totalLength\": " << totalLength << ",\n";
    json << "    \"standardSramLength\": " << Gen1Layout::ExpectedSize << ",\n";
    json << "    \"trailingLength\": " << trailingLength << "\n";
    json << "  },\n";

    json << BuildCoverageJson(coverage);
    json << BuildDecodedJson(save, reader, options.includeDecodedSummary);
    json << BuildConversionModelJson(reader);
    json << BuildUnknownDataJson(coverage);

    json << "  \"reconstruction\": {\n";
    json << "    \"policy\": \"use-physical-image-for-no-edit-reconstruction\",\n";
    json << "    \"semanticFieldsAreInformational\": true,\n";
    json << "    \"noEditRoundTripRequired\": true,\n";
    json << "    \"checksumRepairDuringNoEditReconstruction\": false\n";
    json << "  },\n";
    json << "  \"diagnostics\": {\n";
    json << "    \"coverageUncoveredBytes\": " << coverage.uncoveredBytes << ",\n";
    json << "    \"coverageOverlappingPrimaryBytes\": " << coverage.overlappingPrimaryBytes << ",\n";
    json << "    \"p0Status\": \"raw-image-exportable\"\n";
    json << "  }\n";
    json << "}\n";
    return json.str();
}

} // namespace

std::string RedMasterJsonSha256Hex(const FileManipulation::Bytes& bytes) {
    std::array<unsigned char, CC_SHA256_DIGEST_LENGTH> digest {};
    CC_SHA256(bytes.empty() ? nullptr : bytes.data(),
              static_cast<CC_LONG>(bytes.size()),
              digest.data());

    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (unsigned char b : digest) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

std::string RedBytesToCanonicalHex(const FileManipulation::Bytes& bytes) {
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (u8 b : bytes) {
        oss << std::setw(2) << static_cast<int>(b);
    }
    return oss.str();
}

bool RedCanonicalHexToBytes(const std::string& hex, FileManipulation::Bytes* out, std::string* error) {
    if (hex.size() % 2 != 0) {
        if (error) *error = "Hex payload length is odd";
        return false;
    }

    FileManipulation::Bytes bytes;
    bytes.reserve(hex.size() / 2);
    auto nibble = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };

    for (std::size_t i = 0; i < hex.size(); i += 2) {
        const int hi = nibble(hex[i]);
        const int lo = nibble(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            if (error) {
                std::ostringstream oss;
                oss << "Invalid canonical hex character at position " << i
                    << ". Only uppercase 0-9/A-F without whitespace is allowed.";
                *error = oss.str();
            }
            return false;
        }
        bytes.push_back(static_cast<u8>((hi << 4) | lo));
    }

    if (out) *out = std::move(bytes);
    return true;
}

std::size_t RedCountByteDifferences(
    const FileManipulation::Bytes& a,
    const FileManipulation::Bytes& b,
    std::size_t* firstDifferenceOffset
) {
    if (firstDifferenceOffset) *firstDifferenceOffset = static_cast<std::size_t>(-1);
    const std::size_t minSize = std::min(a.size(), b.size());
    std::size_t count = 0;
    for (std::size_t i = 0; i < minSize; ++i) {
        if (a[i] != b[i]) {
            if (count == 0 && firstDifferenceOffset) *firstDifferenceOffset = i;
            ++count;
        }
    }
    if (a.size() != b.size()) {
        if (count == 0 && firstDifferenceOffset) *firstDifferenceOffset = minSize;
        count += std::max(a.size(), b.size()) - minSize;
    }
    return count;
}

RedCoverageSummary RedCoverageReporter::BuildPrimaryCoverage() {
    RedCoverageSummary s;
    s.totalStandardSramBytes = Gen1Layout::ExpectedSize;
    s.ranges = {
        {0x0000, 0x0497, "Bank 0 sprite/runtime scratch buffers", "runtime_state", false, "preserve-original-bytes", "Runtime/scratch region; preserved losslessly."},
        {0x0498, 0x0597, "Bank 0 unused pre-Hall-of-Fame block", "unused", false, "preserve-original-bytes", "Known unused/padding-style region."},
        {0x0598, 0x1857, "Hall of Fame records", "decoded", true, "preserve-original-bytes", "Decoded by Save Genie; raw bytes remain authoritative for reconstruction."},
        {0x1858, 0x1FFF, "Bank 0 unused post-Hall-of-Fame block", "unused", false, "preserve-original-bytes", "Known unused/padding-style region."},
        {0x2000, 0x2597, "Bank 1 lead-in/runtime region", "runtime_state", false, "preserve-original-bytes", "Runtime/overworld data; only partially interpreted."},
        {0x2598, 0x3522, "Bank 1 main save data checksum-covered range", "partially_decoded", true, "preserve-original-bytes", "Trainer, inventory, world state, events, party, current box cache and related data."},
        {0x3523, 0x3523, "Main checksum byte", "checksum_metadata", true, "preserve-original-bytes", "No-edit reconstruction preserves stored checksum exactly."},
        {0x3524, 0x3FFF, "Bank 1 trailing unknown/raw-preserved region", "unknown", false, "preserve-original-bytes", "Not fully interpreted; preserved losslessly."},
        {0x4000, 0x5A52, "Bank 2 PC Boxes 1-6 and checksum metadata", "decoded", true, "preserve-original-bytes", "Permanent PC storage decoded by Save Genie."},
        {0x5A53, 0x5FFF, "Bank 2 unused/unknown remainder", "unknown", false, "preserve-original-bytes", "Not fully interpreted; preserved losslessly."},
        {0x6000, 0x7A52, "Bank 3 PC Boxes 7-12 and checksum metadata", "decoded", true, "preserve-original-bytes", "Permanent PC storage decoded by Save Genie."},
        {0x7A53, 0x7FFF, "Bank 3 unused/unknown remainder", "unknown", false, "preserve-original-bytes", "Not fully interpreted; preserved losslessly."}
    };

    ValidatePrimaryCoverage(&s, nullptr);
    return s;
}

bool RedCoverageReporter::ValidatePrimaryCoverage(RedCoverageSummary* summary, std::vector<std::string>* errors) {
    if (!summary) return false;

    std::vector<int> hits(Gen1Layout::ExpectedSize, 0);
    summary->decodedBytes = 0;
    summary->partiallyDecodedBytes = 0;
    summary->knownRawBytes = 0;
    summary->runtimeBytes = 0;
    summary->unknownBytes = 0;
    summary->rawPreservedBytes = 0;

    bool ok = true;
    for (const RedCoverageRange& r : summary->ranges) {
        if (r.start > r.endInclusive || r.endInclusive >= Gen1Layout::ExpectedSize) {
            ok = false;
            if (errors) errors->push_back("Invalid coverage range: " + r.name);
            continue;
        }
        const std::size_t len = r.endInclusive - r.start + 1;
        if (r.classification == "decoded") summary->decodedBytes += len;
        else if (r.classification == "partially_decoded") summary->partiallyDecodedBytes += len;
        else if (r.classification == "known_raw" || r.classification == "unused" ||
                 r.classification == "padding" || r.classification == "checksum_metadata") {
            summary->knownRawBytes += len;
        } else if (r.classification == "runtime_state") summary->runtimeBytes += len;
        else if (r.classification == "unknown") summary->unknownBytes += len;
        else if (r.classification == "raw_preserved") summary->rawPreservedBytes += len;

        for (std::size_t i = r.start; i <= r.endInclusive; ++i) {
            ++hits[i];
        }
    }

    summary->uncoveredBytes = 0;
    summary->overlappingPrimaryBytes = 0;
    for (int h : hits) {
        if (h == 0) ++summary->uncoveredBytes;
        if (h > 1) ++summary->overlappingPrimaryBytes;
    }

    if (summary->uncoveredBytes != 0) {
        ok = false;
        if (errors) errors->push_back("Coverage validation failed: uncovered bytes = " + std::to_string(summary->uncoveredBytes));
    }
    if (summary->overlappingPrimaryBytes != 0) {
        ok = false;
        if (errors) errors->push_back("Coverage validation failed: overlapping primary bytes = " + std::to_string(summary->overlappingPrimaryBytes));
    }

    return ok;
}

std::string RedMasterJsonExporter::MakeRedJsonPathCollisionSafe(const std::string& savePath) {
    namespace fs = std::filesystem;
    const fs::path p(savePath);
    const fs::path dir = p.parent_path();
    const std::string stem = p.stem().string();

    for (int i = 1; i < 10000; ++i) {
        std::ostringstream name;
        name << stem;
        if (i > 1) name << " " << i;
        name << ".red.json";
        const fs::path candidate = dir / name.str();
        if (!FileExists(candidate)) return candidate.string();
    }
    throw std::runtime_error("Could not create collision-safe .red.json path for " + savePath);
}

RedMasterJsonResult RedMasterJsonExporter::ExportToFile(
    const std::string& inputPath,
    const SaveBuffer& save,
    const ReadOnlyData& reader,
    const RedMasterJsonOptions& options
) {
    RedMasterJsonResult result;
    result.originalPath = inputPath;
    result.originalSize = save.Size();
    result.coverage = RedCoverageReporter::BuildPrimaryCoverage();

    if (!RedCoverageReporter::ValidatePrimaryCoverage(&result.coverage, &result.errors)) {
        return result;
    }
    if (save.Size() < Gen1Layout::ExpectedSize) {
        result.errors.push_back("Cannot export .red.json: source file is shorter than 0x8000 bytes.");
        return result;
    }

    try {
        const std::string jsonPath = MakeRedJsonPathCollisionSafe(inputPath);
        const std::string json = BuildMasterJson(inputPath, save, reader, options, result.coverage, &result.warnings);
        FileManipulation::Bytes out(json.begin(), json.end());
        FileManipulation::WriteFile(jsonPath, out);
        result.ok = true;
        result.outputPath = jsonPath;
        result.jsonPath = jsonPath;
        result.wholeFileSha256 = RedMasterJsonSha256Hex(save.BytesView());
    } catch (const std::exception& e) {
        result.errors.push_back(e.what());
    }

    return result;
}

RedMasterJsonDocument RedMasterJsonImporter::LoadAndValidate(const std::string& jsonPath) {
    RedMasterJsonDocument doc;
    FileManipulation::Bytes jsonBytes;
    try {
        jsonBytes = FileManipulation::LoadFile(jsonPath);
    } catch (const std::exception& e) {
        doc.errors.push_back(e.what());
        return doc;
    }

    const std::string text(jsonBytes.begin(), jsonBytes.end());
    JsonValue root;
    std::string parseError;
    JsonParser parser(text);
    if (!parser.Parse(&root, &parseError)) {
        doc.errors.push_back("JSON syntax validation failed: " + parseError);
        return doc;
    }
    if (root.type != JsonValue::Type::Object) {
        doc.errors.push_back("Root JSON value must be an object.");
        return doc;
    }

    const JsonValue* schema = nullptr;
    const JsonValue* source = nullptr;
    const JsonValue* physical = nullptr;
    if (!RequireObject(root, "schema", &schema, &doc.errors) ||
        !RequireObject(root, "source", &source, &doc.errors) ||
        !RequireObject(root, "physicalImage", &physical, &doc.errors)) {
        return doc;
    }

    std::string format;
    std::string version;
    if (!RequireString(*schema, "format", &format, &doc.errors) ||
        !RequireString(*schema, "schemaVersion", &version, &doc.errors)) {
        return doc;
    }
    if (format != kFormat) {
        doc.errors.push_back("Unsupported schema format: " + format);
        return doc;
    }
    if (version != kSchemaVersion) {
        doc.errors.push_back("Unsupported schema version: " + version + " (supported: " + kSchemaVersion + ")");
        return doc;
    }

    RequireString(*source, "fileName", &doc.originalFileName, &doc.errors);
    const JsonValue* hashes = nullptr;
    if (!RequireObject(*source, "hashes", &hashes, &doc.errors)) return doc;
    RequireString(*hashes, "wholeFileSha256", &doc.wholeFileSha256, &doc.errors);
    RequireString(*hashes, "standardSramSha256", &doc.standardSramSha256, &doc.errors);
    if (const JsonValue* trailingHash = ObjectField(*hashes, "trailingDataSha256")) {
        if (trailingHash->type == JsonValue::Type::String) {
            doc.trailingDataSha256 = trailingHash->text;
        } else if (trailingHash->type != JsonValue::Type::Null) {
            doc.errors.push_back("trailingDataSha256 must be a string or null.");
        }
    } else {
        doc.errors.push_back("Missing trailingDataSha256 field.");
    }

    std::string encoding;
    std::string standardHex;
    std::string trailingHex;
    if (!RequireString(*physical, "encoding", &encoding, &doc.errors) ||
        !RequireString(*physical, "standardSramHex", &standardHex, &doc.errors) ||
        !RequireString(*physical, "trailingDataHex", &trailingHex, &doc.errors) ||
        !RequireSize(*physical, "totalLength", &doc.declaredTotalLength, &doc.errors) ||
        !RequireSize(*physical, "standardSramLength", &doc.declaredStandardSramLength, &doc.errors) ||
        !RequireSize(*physical, "trailingLength", &doc.declaredTrailingLength, &doc.errors)) {
        return doc;
    }

    if (encoding != kHexEncoding) {
        doc.errors.push_back("Unsupported physicalImage encoding: " + encoding);
        return doc;
    }

    FileManipulation::Bytes standard;
    FileManipulation::Bytes trailing;
    std::string hexError;
    if (!RedCanonicalHexToBytes(standardHex, &standard, &hexError)) {
        doc.errors.push_back("Invalid standardSramHex: " + hexError);
        return doc;
    }
    if (!RedCanonicalHexToBytes(trailingHex, &trailing, &hexError)) {
        doc.errors.push_back("Invalid trailingDataHex: " + hexError);
        return doc;
    }

    if (doc.declaredStandardSramLength != Gen1Layout::ExpectedSize ||
        standard.size() != Gen1Layout::ExpectedSize) {
        doc.errors.push_back("standardSramHex must decode to exactly 0x8000 bytes.");
        return doc;
    }
    if (trailing.size() != doc.declaredTrailingLength) {
        doc.errors.push_back("trailingDataHex decoded length does not match trailingLength.");
        return doc;
    }
    if (standard.size() + trailing.size() != doc.declaredTotalLength) {
        doc.errors.push_back("Decoded physical image length does not match totalLength.");
        return doc;
    }

    doc.bytes = std::move(standard);
    doc.bytes.insert(doc.bytes.end(), trailing.begin(), trailing.end());

    const FileManipulation::Bytes standardCheck = SliceBytes(doc.bytes, 0, Gen1Layout::ExpectedSize);
    const FileManipulation::Bytes trailingCheck = SliceBytes(doc.bytes, Gen1Layout::ExpectedSize, trailing.size());
    const std::string wholeHash = RedMasterJsonSha256Hex(doc.bytes);
    const std::string standardHash = RedMasterJsonSha256Hex(standardCheck);
    const std::string trailingHash = trailingCheck.empty() ? std::string() : RedMasterJsonSha256Hex(trailingCheck);

    if (wholeHash != doc.wholeFileSha256) {
        doc.errors.push_back("Whole-file SHA-256 mismatch.");
    }
    if (standardHash != doc.standardSramSha256) {
        doc.errors.push_back("Standard SRAM SHA-256 mismatch.");
    }
    if (!trailingCheck.empty() && trailingHash != doc.trailingDataSha256) {
        doc.errors.push_back("Trailing data SHA-256 mismatch.");
    }
    if (trailingCheck.empty() && !doc.trailingDataSha256.empty()) {
        doc.errors.push_back("Trailing data hash is present but trailingLength is zero.");
    }

    return doc;
}

std::string RedSaveReconstructor::MakeReconstructedPathCollisionSafe(const std::string& originalSavePath) {
    namespace fs = std::filesystem;
    fs::path p(originalSavePath);
    const fs::path dir = p.parent_path();
    const std::string filename = p.filename().string();
    return CollisionSafePathWithPrefix(dir, "[RECONSTRUCTED] ", filename);
}

RedMasterJsonResult RedSaveReconstructor::ReconstructToFile(
    const std::string& jsonPath,
    const std::string& originalSavePath
) {
    RedMasterJsonResult result;
    result.jsonPath = jsonPath;
    result.originalPath = originalSavePath;

    const RedMasterJsonDocument doc = RedMasterJsonImporter::LoadAndValidate(jsonPath);
    result.originalFileName = doc.originalFileName;
    result.originalSize = doc.declaredTotalLength;
    result.wholeFileSha256 = doc.wholeFileSha256;
    result.errors = doc.errors;
    result.warnings = doc.warnings;
    if (!result.errors.empty()) {
        return result;
    }

    const std::string basePath = originalSavePath.empty() ? doc.originalFileName : originalSavePath;
    const std::string outPath = MakeReconstructedPathCollisionSafe(basePath);
    try {
        FileManipulation::WriteFile(outPath, doc.bytes);
    } catch (const std::exception& e) {
        result.errors.push_back(e.what());
        return result;
    }

    result.outputPath = outPath;
    result.reconstructedSize = doc.bytes.size();
    result.reconstructedSha256 = RedMasterJsonSha256Hex(doc.bytes);

    if (!originalSavePath.empty()) {
        try {
            const FileManipulation::Bytes original = FileManipulation::LoadFile(originalSavePath);
            result.originalSize = original.size();
            result.wholeFileSha256 = RedMasterJsonSha256Hex(original);
            result.byteDifferenceCount = RedCountByteDifferences(original, doc.bytes, &result.firstDifferenceOffset);
        } catch (const std::exception& e) {
            result.warnings.push_back(std::string("Could not compare against original file: ") + e.what());
            result.byteDifferenceCount = (result.reconstructedSha256 == doc.wholeFileSha256) ? 0 : 1;
        }
    } else {
        result.byteDifferenceCount = (result.reconstructedSha256 == doc.wholeFileSha256) ? 0 : 1;
    }

    if (result.reconstructedSize != doc.declaredTotalLength) {
        result.errors.push_back("Reconstructed size does not match declared totalLength.");
    }
    if (result.reconstructedSha256 != doc.wholeFileSha256) {
        result.errors.push_back("Reconstructed SHA-256 does not match .red.json wholeFileSha256.");
    }
    if (result.byteDifferenceCount != 0) {
        result.errors.push_back("Reconstructed bytes are not byte-identical to the original source.");
    }

    result.ok = result.errors.empty();
    return result;
}

} // namespace savegenie
