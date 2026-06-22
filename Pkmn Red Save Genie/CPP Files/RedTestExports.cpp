//
//  RedTestExports.cpp
//  Pkmn Red Save Genie
//
//  Legacy/test JSON and text exports kept separate from RedMasterJson.
//

#include "RedTestExports.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace savegenie {

std::string JsonEscape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) out += '?';
                else out += c;
                break;
        }
    }
    return out;
}

static std::string ToHexByteString(u8 v) {
    std::ostringstream oss;
    oss << "0x" << std::uppercase << std::hex
        << std::setw(2) << std::setfill('0')
        << static_cast<int>(v)
        << std::nouppercase << std::dec;
    return oss.str();
}

static void WritePokemonBoxJsonObject(std::ofstream& f, const PokemonBox& box, const std::string& indent) {
    f << indent << "{\n";
    f << indent << "  \"boxNumber\": " << box.boxNumber << ",\n";
    f << indent << "  \"label\": \"" << JsonEscape(box.label) << "\",\n";
    f << indent << "  \"pokemonCount\": " << box.pokemonCount << ",\n";
    f << indent << "  \"pokemon\": [\n";

    for (std::size_t mi = 0; mi < box.pokemon.size(); ++mi) {
        const auto& mon = box.pokemon[mi];
        f << indent << "    {\n";
        f << indent << "      \"position\": " << mon.position << ",\n";
        f << indent << "      \"speciesId\": " << static_cast<int>(mon.speciesId) << ",\n";
        f << indent << "      \"speciesName\": \"" << JsonEscape(mon.speciesName) << "\",\n";
        f << indent << "      \"dexNo\": \"" << JsonEscape(mon.dexNo) << "\",\n";
        f << indent << "      \"nickname\": \"" << JsonEscape(mon.nickname) << "\",\n";
        f << indent << "      \"otName\": \"" << JsonEscape(mon.otName) << "\",\n";
        f << indent << "      \"otIdNo\": " << mon.otIdNo << ",\n";
        f << indent << "      \"level\": " << static_cast<int>(mon.level) << ",\n";
        f << indent << "      \"expPoints\": " << mon.expPoints << ",\n";

        f << indent << "      \"dvs\": { "
          << "\"hp\": " << static_cast<int>(mon.dvHP) << ", "
          << "\"atk\": " << static_cast<int>(mon.dvAtk) << ", "
          << "\"def\": " << static_cast<int>(mon.dvDef) << ", "
          << "\"spd\": " << static_cast<int>(mon.dvSpd) << ", "
          << "\"spc\": " << static_cast<int>(mon.dvSpc) << " },\n";

        f << indent << "      \"statExp\": { "
          << "\"hp\": " << mon.statExpHP << ", "
          << "\"atk\": " << mon.statExpAtk << ", "
          << "\"def\": " << mon.statExpDef << ", "
          << "\"spd\": " << mon.statExpSpd << ", "
          << "\"spc\": " << mon.statExpSpc << " },\n";

        f << indent << "      \"stats\": { "
          << "\"hpCurrent\": " << mon.stats.hpCurrent << ", "
          << "\"hpMax\": " << mon.stats.hpMax << ", "
          << "\"attack\": " << mon.stats.attack << ", "
          << "\"defense\": " << mon.stats.defense << ", "
          << "\"speed\": " << mon.stats.speed << ", "
          << "\"special\": " << mon.stats.special << ", "
          << "\"statusHex\": \"" << ToHexByteString(mon.stats.status) << "\" },\n";

        f << indent << "      \"moves\": [\n";
        const std::size_t moveCount = std::min<std::size_t>(4, mon.moves.size());
        for (std::size_t mv = 0; mv < moveCount; ++mv) {
            const auto& m = mon.moves[mv];
            f << indent << "        { "
              << "\"moveId\": " << static_cast<int>(m.moveId) << ", "
              << "\"moveName\": \"" << JsonEscape(m.moveName) << "\", "
              << "\"ppCurrent\": " << static_cast<int>(m.ppCurrent) << ", "
              << "\"ppMax\": " << static_cast<int>(m.ppMax)
              << " }";
            if (mv + 1 < moveCount) f << ",";
            f << "\n";
        }
        f << indent << "      ]\n";

        f << indent << "    }";
        if (mi + 1 < box.pokemon.size()) f << ",";
        f << "\n";
    }

    f << indent << "  ]\n";
    f << indent << "}";
}

bool WritePokemonBoxesJson(const PokemonBoxesExport& ex, const std::string& outPath) {
    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;

    f << "{\n";
    f << "  \"game\": \"Pokemon Red (Gen I)\",\n";
    f << "  \"boxes\": [\n";

    for (std::size_t bi = 0; bi < ex.boxes.size(); ++bi) {
        WritePokemonBoxJsonObject(f, ex.boxes[bi], "    ");
        if (bi + 1 < ex.boxes.size()) f << ",";
        f << "\n";
    }

    f << "  ],\n";
    f << "  \"currentBoxCache\": ";
    if (ex.hasCurrentBoxCache) {
        WritePokemonBoxJsonObject(f, ex.currentBoxCache, "  ");
        f << "\n";
    } else {
        f << "null\n";
    }
    f << "}\n";

    return true;
}

bool WritePokemonSummaryJson(
    const ReadOnlyData& reader,
    const std::string& inputPath,
    const std::string& backupPath,
    std::size_t saveSize,
    bool sizeLooksExpected,
    bool mainChecksumValid,
    bool bank2AllValid,
    bool bank3AllValid,
    const std::string& outPath
) {
    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;

    const TrainerSummary trainer = reader.GetTrainerSummary();
    const PokedexSummary pokedex = reader.GetPokedexSummary(false);
    const BagSummary bag = reader.GetBagSummary(false);
    const BagSummary pcItems = reader.GetPCItemBoxSummary(false);
    const FlagSummary eventFlags = reader.GetEventFlagSummary();
    const PokemonBox currentBoxCache = reader.GetCurrentBoxCache();
    const PlayerStateSummary playerState = reader.GetPlayerStateSummary();
    const WorldStateSummary worldState = reader.GetWorldStateSummary();
    const EventCategorySummary eventCategories = reader.GetEventCategorySummary();
    const DaycareSummary daycare = reader.GetDaycareSummary();
    const PokemonBox party = reader.GetPartyAsBox0();

    int boxCounts[12] = {0};
    for (int i = 1; i <= 12; ++i) {
        try {
            const PokemonBox b = reader.GetPCBox(i);
            boxCounts[i - 1] = b.pokemonCount;
        } catch (...) {
            boxCounts[i - 1] = 0;
        }
    }

    f << "{\n";
    f << "  \"game\": \"Pokemon Red (Gen I)\",\n";

    f << "  \"file\": {\n";
    f << "    \"input\": \"" << JsonEscape(inputPath) << "\",\n";
    f << "    \"backup\": \"" << JsonEscape(backupPath) << "\",\n";
    f << "    \"sizeBytes\": " << static_cast<unsigned long long>(saveSize) << ",\n";
    f << "    \"sizeHex\": \"0x" << std::hex << saveSize << std::dec << "\",\n";
    f << "    \"expected32KB\": " << (sizeLooksExpected ? "true" : "false") << "\n";
    f << "  },\n";

    f << "  \"checksums\": {\n";
    f << "    \"mainValid\": " << (mainChecksumValid ? "true" : "false") << ",\n";
    f << "    \"bank2AllValid\": " << (bank2AllValid ? "true" : "false") << ",\n";
    f << "    \"bank3AllValid\": " << (bank3AllValid ? "true" : "false") << "\n";
    f << "  },\n";

    f << "  \"trainer\": {\n";
    f << "    \"name\": \"" << JsonEscape(trainer.trainerName) << "\",\n";
    f << "    \"rivalName\": \"" << JsonEscape(trainer.rivalName) << "\",\n";
    f << "    \"trainerId\": " << trainer.trainerId << ",\n";
    f << "    \"money\": " << trainer.money << ",\n";
    f << "    \"coins\": " << trainer.coins << ",\n";
    f << "    \"badgesBitfield\": " << static_cast<int>(trainer.badges) << "\n";
    f << "  },\n";

    f << "  \"location\": {\n";
    f << "    \"mapId\": " << static_cast<int>(trainer.mapId) << ",\n";
    f << "    \"mapHex\": \"" << Gen1MapLookup::MapIDHex[static_cast<int>(trainer.mapId)] << "\",\n";
    f << "    \"mapName\": \"" << JsonEscape(Gen1MapLookup::MapIDName[static_cast<int>(trainer.mapId)]) << "\",\n";
    f << "    \"x\": " << static_cast<int>(trainer.x) << ",\n";
    f << "    \"y\": " << static_cast<int>(trainer.y) << "\n";
    f << "  },\n";

    f << "  \"playerState\": {\n";
    f << "    \"optionsByte\": " << static_cast<int>(playerState.optionsByte) << ",\n";
    f << "    \"letterDelayByte\": " << static_cast<int>(playerState.letterDelayByte) << ",\n";
    f << "    \"contrast\": " << static_cast<int>(playerState.contrast) << ",\n";
    f << "    \"xBlockCoord\": " << static_cast<int>(playerState.xBlockCoord) << ",\n";
    f << "    \"yBlockCoord\": " << static_cast<int>(playerState.yBlockCoord) << ",\n";
    f << "    \"movementMode\": \"" << JsonEscape(playerState.MovementModeName()) << "\",\n";
    f << "    \"safariGameOver\": " << (playerState.safariGameOver ? "true" : "false") << ",\n";
    f << "    \"safariBallCount\": " << static_cast<int>(playerState.safariBallCount) << ",\n";
    f << "    \"safariSteps\": " << playerState.safariSteps << "\n";
    f << "  },\n";

    f << "  \"playtime\": {\n";
    f << "    \"hours\": " << static_cast<int>(trainer.playHours) << ",\n";
    f << "    \"minutes\": " << static_cast<int>(trainer.playMinutes) << ",\n";
    f << "    \"seconds\": " << static_cast<int>(trainer.playSeconds) << "\n";
    f << "  },\n";

    f << "  \"pokedex\": {\n";
    f << "    \"ownedCount\": " << pokedex.ownedCount << ",\n";
    f << "    \"seenCount\": " << pokedex.seenCount << "\n";
    f << "  },\n";

    f << "  \"inventory\": {\n";
    f << "    \"bagCount\": " << bag.itemCount << ",\n";
    f << "    \"pcItemCount\": " << pcItems.itemCount << "\n";
    f << "  },\n";

    f << "  \"events\": {\n";
    f << "    \"flagsSet\": " << eventFlags.totalFlagsSet << ",\n";
    f << "    \"namedFlagsKnown\": " << eventFlags.namedFlagsKnown << ",\n";
    f << "    \"namedFlagsSet\": " << eventFlags.namedFlagsSet << ",\n";
    f << "    \"beatFlagsSet\": " << eventFlags.beatFlagsSet << "\n";
    f << "  },\n";

    f << "  \"eventCategories\": {\n";
    f << "    \"defeatedTrainerFlagsSet\": " << eventCategories.defeatedTrainerFlagsSet << ",\n";
    f << "    \"trainerFlagsKnown\": " << eventCategories.trainerFlagsKnown << ",\n";
    f << "    \"trainerFlagsComplete\": " << eventCategories.trainerFlagsComplete << ",\n";
    f << "    \"storyFlagsKnown\": " << eventCategories.storyFlagsKnown << ",\n";
    f << "    \"storyFlagsComplete\": " << eventCategories.storyFlagsComplete << ",\n";
    f << "    \"staticEncounterFlagsKnown\": " << eventCategories.staticEncounterFlagsKnown << ",\n";
    f << "    \"staticEncounterFlagsComplete\": " << eventCategories.staticEncounterFlagsComplete << ",\n";
    f << "    \"gymLeaderFlagsSet\": " << eventCategories.gymLeaderFlagsSet << ",\n";
    f << "    \"majorStoryMilestonesSet\": " << eventCategories.majorStoryMilestonesSet << ",\n";
    f << "    \"legendaryFlagsSet\": " << eventCategories.legendaryFlagsSet << ",\n";
    f << "    \"gymStoryMismatchCount\": " << eventCategories.gymStoryMismatchCount << ",\n";
    f << "    \"trainerFlags\": [\n";
    for (std::size_t i = 0; i < eventCategories.trainerFlags.size(); ++i) {
        const auto& flag = eventCategories.trainerFlags[i];
        f << "      { "
          << "\"label\": \"" << JsonEscape(flag.label) << "\", "
          << "\"location\": \"" << JsonEscape(flag.location) << "\", "
          << "\"trainerNumber\": " << flag.trainerNumber << ", "
          << "\"eventName\": \"" << JsonEscape(flag.eventName) << "\", "
          << "\"flagIndex\": " << flag.index << ", "
          << "\"complete\": " << (flag.isComplete ? "true" : "false")
          << " }";
        if (i + 1 < eventCategories.trainerFlags.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"staticEncounterFlags\": [\n";
    for (std::size_t i = 0; i < eventCategories.staticEncounterFlags.size(); ++i) {
        const auto& flag = eventCategories.staticEncounterFlags[i];
        f << "      { "
          << "\"label\": \"" << JsonEscape(flag.label) << "\", "
          << "\"category\": \"" << JsonEscape(flag.category) << "\", "
          << "\"eventName\": \"" << JsonEscape(flag.eventName) << "\", "
          << "\"flagIndex\": " << flag.index << ", "
          << "\"complete\": " << (flag.isComplete ? "true" : "false")
          << " }";
        if (i + 1 < eventCategories.staticEncounterFlags.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"storyFlags\": [\n";
    for (std::size_t i = 0; i < eventCategories.storyFlags.size(); ++i) {
        const auto& flag = eventCategories.storyFlags[i];
        f << "      { "
          << "\"label\": \"" << JsonEscape(flag.label) << "\", "
          << "\"category\": \"" << JsonEscape(flag.category) << "\", "
          << "\"eventName\": \"" << JsonEscape(flag.eventName) << "\", "
          << "\"flagIndex\": " << flag.index << ", "
          << "\"complete\": " << (flag.isComplete ? "true" : "false")
          << " }";
        if (i + 1 < eventCategories.storyFlags.size()) f << ",";
        f << "\n";
    }
    f << "    ]\n";
    f << "  },\n";

    f << "  \"worldState\": {\n";
    f << "    \"missableObjectsSet\": " << worldState.missableObjectsSet << ",\n";
    f << "    \"missableObjectsChecked\": " << worldState.missableObjectsChecked << ",\n";
    f << "    \"hiddenItemsCollected\": " << worldState.hiddenItemsCollected << ",\n";
    f << "    \"hiddenItemsChecked\": " << worldState.hiddenItemsChecked << ",\n";
    f << "    \"hiddenCoinsCollected\": " << worldState.hiddenCoinsCollected << ",\n";
    f << "    \"hiddenCoinsChecked\": " << worldState.hiddenCoinsChecked << ",\n";
    f << "    \"visitedTownsSet\": " << worldState.visitedTownsSet << ",\n";
    f << "    \"visitedTownsChecked\": " << worldState.visitedTownsChecked << ",\n";
    f << "    \"currentScriptBytesNonZero\": " << worldState.currentScriptsNonZero << ",\n";
    f << "    \"currentScriptBytesChecked\": " << worldState.currentScriptsChecked << ",\n";
    f << "    \"currentScripts\": [\n";
    for (std::size_t i = 0; i < worldState.currentScripts.size(); ++i) {
        const auto& script = worldState.currentScripts[i];
        f << "      { "
          << "\"index\": " << script.index << ", "
          << "\"name\": \"" << JsonEscape(script.name) << "\", "
          << "\"relativeOffset\": " << script.relativeOffset << ", "
          << "\"size\": " << script.size << ", "
          << "\"value\": " << script.value
          << " }";
        if (i + 1 < worldState.currentScripts.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"missableObjects\": [\n";
    for (std::size_t i = 0; i < worldState.missableObjects.size(); ++i) {
        const auto& item = worldState.missableObjects[i];
        f << "      { "
          << "\"index\": " << item.index << ", "
          << "\"name\": \"" << JsonEscape(item.name) << "\", "
          << "\"location\": \"" << JsonEscape(item.location) << "\", "
          << "\"sprite\": " << item.sprite << ", "
          << "\"defaultState\": \"" << JsonEscape(item.defaultState) << "\", "
          << "\"toggledOff\": " << (item.isSet ? "true" : "false")
          << " }";
        if (i + 1 < worldState.missableObjects.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"hiddenItems\": [\n";
    for (std::size_t i = 0; i < worldState.hiddenItems.size(); ++i) {
        const auto& item = worldState.hiddenItems[i];
        f << "      { "
          << "\"index\": " << item.index << ", "
          << "\"location\": \"" << JsonEscape(item.location) << "\", "
          << "\"x\": " << item.x << ", "
          << "\"y\": " << item.y << ", "
          << "\"collected\": " << (item.isSet ? "true" : "false")
          << " }";
        if (i + 1 < worldState.hiddenItems.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"hiddenCoins\": [\n";
    for (std::size_t i = 0; i < worldState.hiddenCoins.size(); ++i) {
        const auto& item = worldState.hiddenCoins[i];
        f << "      { "
          << "\"index\": " << item.index << ", "
          << "\"location\": \"" << JsonEscape(item.location) << "\", "
          << "\"x\": " << item.x << ", "
          << "\"y\": " << item.y << ", "
          << "\"collected\": " << (item.isSet ? "true" : "false")
          << " }";
        if (i + 1 < worldState.hiddenCoins.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"visitedTowns\": [\n";
    for (std::size_t i = 0; i < worldState.visitedTowns.size(); ++i) {
        const auto& item = worldState.visitedTowns[i];
        f << "      { "
          << "\"index\": " << item.index << ", "
          << "\"name\": \"" << JsonEscape(item.name) << "\", "
          << "\"visited\": " << (item.isSet ? "true" : "false")
          << " }";
        if (i + 1 < worldState.visitedTowns.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"runtimeFields\": [\n";
    for (std::size_t i = 0; i < worldState.runtimeFields.size(); ++i) {
        const auto& field = worldState.runtimeFields[i];
        f << "      { "
          << "\"offset\": \"" << JsonEscape(field.offsetHex) << "\", "
          << "\"name\": \"" << JsonEscape(field.name) << "\", "
          << "\"value\": \"" << JsonEscape(field.value) << "\", "
          << "\"source\": \"" << JsonEscape(field.source) << "\""
          << " }";
        if (i + 1 < worldState.runtimeFields.size()) f << ",";
        f << "\n";
    }
    f << "    ],\n";
    f << "    \"gotStarter\": " << (worldState.gotStarter ? "true" : "false") << ",\n";
    f << "    \"gotLapras\": " << (worldState.gotLapras ? "true" : "false") << ",\n";
    f << "    \"defeatedLorelei\": " << (worldState.defeatedLorelei ? "true" : "false") << "\n";
    f << "  },\n";

    f << "  \"daycare\": {\n";
    f << "    \"inUse\": " << (daycare.inUse ? "true" : "false") << ",\n";
    f << "    \"pokemon\": ";
    if (daycare.inUse) {
        f << "{ "
          << "\"speciesId\": " << static_cast<int>(daycare.pokemon.speciesId) << ", "
          << "\"speciesName\": \"" << JsonEscape(daycare.pokemon.speciesName) << "\", "
          << "\"dexNo\": \"" << JsonEscape(daycare.pokemon.dexNo) << "\", "
          << "\"nickname\": \"" << JsonEscape(daycare.pokemon.nickname) << "\", "
          << "\"otName\": \"" << JsonEscape(daycare.pokemon.otName) << "\", "
          << "\"level\": " << static_cast<int>(daycare.pokemon.level)
          << " }\n";
    } else {
        f << "null\n";
    }
    f << "  },\n";

    f << "  \"party\": {\n";
    f << "    \"count\": " << party.pokemonCount << ",\n";
    f << "    \"pokemon\": [\n";
    for (std::size_t i = 0; i < party.pokemon.size(); ++i) {
        const auto& mon = party.pokemon[i];
        f << "      {\n";
        f << "        \"position\": " << mon.position << ",\n";
        f << "        \"speciesId\": " << static_cast<int>(mon.speciesId) << ",\n";
        f << "        \"speciesName\": \"" << JsonEscape(mon.speciesName) << "\",\n";
        f << "        \"dexNo\": \"" << JsonEscape(mon.dexNo) << "\",\n";
        f << "        \"nickname\": \"" << JsonEscape(mon.nickname) << "\",\n";
        f << "        \"otName\": \"" << JsonEscape(mon.otName) << "\",\n";
        f << "        \"otIdNo\": " << mon.otIdNo << ",\n";
        f << "        \"level\": " << static_cast<int>(mon.level) << "\n";
        f << "      }";
        if (i + 1 < party.pokemon.size()) f << ",";
        f << "\n";
    }
    f << "    ]\n";
    f << "  },\n";

    f << "  \"pcBoxes\": {\n";
    f << "    \"counts\": [";
    for (int i = 0; i < 12; ++i) {
        f << boxCounts[i];
        if (i != 11) f << ", ";
    }
    f << "],\n";
    f << "    \"currentBoxCacheCount\": " << currentBoxCache.pokemonCount << "\n";
    f << "  },\n";

    f << "  \"outputs\": {\n";
    f << "    \"boxesJson\": \"PokemonBoxes.json\",\n";
    f << "    \"summaryTxt\": \"SaveGenieSummary.txt\"\n";
    f << "  }\n";

    f << "}\n";
    return true;
}

bool WriteSaveGenieSummaryTxt(const std::string& text, const std::string& outPath) {
    std::ofstream f(outPath, std::ios::binary);
    if (!f) return false;
    f.write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(f);
}

} // namespace savegenie
