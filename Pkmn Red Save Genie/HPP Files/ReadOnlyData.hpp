//
//  ReadOnlyData.hpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - High-level, human-readable extraction layer.
//   - Converts raw save bytes (via SaveStructure) into plain English summaries.
//   - Contains NO write/edit logic.
//
//  Owns:
//   - Trainer summary extraction
//   - Location summary extraction
//   - Money / Coins display
//   - Badge interpretation
//   - Playtime formatting
//   - Box statistics (count / average level)
//   - Basic flag summaries
//
//  Does NOT:
//   - Modify save data
//   - Perform file I/O
//   - Recalculate checksums
//

#ifndef ReadOnlyData_hpp
#define ReadOnlyData_hpp

#include <string>
#include <vector>
#include <cstdint>

#include "SaveStructure.hpp"

namespace savegenie {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

// =========================================================
// Trainer Summary Model
// =========================================================

class TrainerSummary {
public:
    std::string trainerName;
    std::string rivalName;

    u16 trainerId = 0;

    u32 money = 0;
    u16 coins = 0;

    u8 badges = 0;

    u8 mapId = 0;
    u8 x = 0;
    u8 y = 0;

    u8 playHours = 0;
    u8 playMinutes = 0;
    u8 playSeconds = 0;

    std::string ToString() const;
};

// =========================================================
// Box Statistics Model
// =========================================================

class BoxStats {
public:
    int boxIndex = 0;           // 1..12
    int pokemonCount = 0;
    double averageLevel = 0.0;

    std::string ToString() const;
};

// =========================================================
// Flag Summary Model
// =========================================================

class FlagSummary {
public:
    class NamedFlag {
    public:
        int index = 0;
        std::string name;
        bool isSet = false;
    };

    int totalFlagsChecked = 0;
    int totalFlagsSet = 0;
    int namedFlagsKnown = 0;
    int namedFlagsSet = 0;
    int beatFlagsSet = 0;

    std::vector<int> setFlagIndices;
    std::vector<NamedFlag> namedFlags;
    std::vector<NamedFlag> namedSetFlags;

    std::string ToString() const;
};

// =========================================================
// Event / Story Category Summary Model
// =========================================================

class EventCategorySummary {
public:
    class TrainerFlag {
    public:
        int index = 0;
        std::string eventName;
        std::string label;
        std::string location;
        int trainerNumber = 0;
        bool isComplete = false;
    };

    class StoryFlag {
    public:
        int index = 0;
        std::string eventName;
        std::string label;
        std::string category;
        bool isComplete = false;
    };

    class GymConsistency {
    public:
        std::string badgeName;
        std::string eventName;
        bool badgeOwned = false;
        bool leaderEventSet = false;
        bool consistent = true;
    };

    int defeatedTrainerFlagsSet = 0;
    int trainerFlagsKnown = 0;
    int trainerFlagsComplete = 0;
    int storyFlagsKnown = 0;
    int storyFlagsComplete = 0;
    int staticEncounterFlagsKnown = 0;
    int staticEncounterFlagsComplete = 0;
    int gymLeaderFlagsSet = 0;
    int majorStoryMilestonesSet = 0;
    int legendaryFlagsSet = 0;
    int gymStoryMismatchCount = 0;

    std::vector<std::string> defeatedTrainerFlags;
    std::vector<std::string> majorStoryMilestones;
    std::vector<std::string> legendaryFlags;
    std::vector<TrainerFlag> trainerFlags;
    std::vector<StoryFlag> storyFlags;
    std::vector<StoryFlag> staticEncounterFlags;
    std::vector<GymConsistency> gymConsistency;

    std::string ToString() const;
};

// =========================================================
// Player / Runtime State Summary Model
// =========================================================

class PlayerStateSummary {
public:
    u8 optionsByte = 0;
    u8 letterDelayByte = 0;
    u8 contrast = 0;

    u8 yBlockCoord = 0;
    u8 xBlockCoord = 0;
    u8 playerMoveDir = 0;
    u8 playerLastStopDir = 0;
    u8 playerCurDir = 0;
    u8 walkBikeSurf = 0;
    u8 playerJumpingYScreen = 0;
    u8 specialWarpY = 0;
    u8 specialWarpX = 0;

    u16 safariSteps = 0;
    u8 safariBallCount = 0;
    bool safariGameOver = false;

    bool strengthOutsideBattle = false;
    bool surfingAllowed = false;
    bool flyOutOfBattle = false;
    bool usedCardKey = false;
    bool isBattle = false;
    bool isTrainerBattle = false;
    bool noBattles = false;
    bool battleEndedOrBlackout = false;
    bool usingLinkCable = false;
    bool standingOnDoor = false;
    bool movingThroughDoor = false;
    bool standingOnWarp = false;
    bool finalLedgeJumping = false;
    bool spinPlayer = false;
    bool noLetterDelay = false;
    bool countPlaytime = false;

    std::string MovementModeName() const;
    std::string DirectionName(u8 dir) const;
    std::string ToString() const;
};

// =========================================================
// World / Object State Summary Model
// =========================================================

class WorldStateSummary {
public:
    class NamedBitState {
    public:
        int index = 0;
        std::string name;
        std::string location;
        int x = -1;
        int y = -1;
        int sprite = -1;
        std::string defaultState;
        bool isSet = false;
    };

    class CurrentScriptState {
    public:
        int index = 0;
        std::string name;
        int relativeOffset = 0;
        int size = 0;
        int value = 0;
    };

    class RuntimeField {
    public:
        std::string name;
        std::string offsetHex;
        std::string value;
        std::string source;
    };

    int missableObjectsChecked = 0;
    int missableObjectsSet = 0;
    int hiddenItemsChecked = 0;
    int hiddenItemsCollected = 0;
    int hiddenCoinsChecked = 0;
    int hiddenCoinsCollected = 0;
    int visitedTownsChecked = 0;
    int visitedTownsSet = 0;
    int currentScriptsChecked = 0;
    int currentScriptsNonZero = 0;

    std::vector<int> firstSetMissableObjects;
    std::vector<int> firstCollectedHiddenItems;
    std::vector<int> firstCollectedHiddenCoins;
    std::vector<int> visitedTownIndices;
    std::vector<int> nonZeroCurrentScriptIndices;

    std::vector<NamedBitState> missableObjects;
    std::vector<NamedBitState> hiddenItems;
    std::vector<NamedBitState> hiddenCoins;
    std::vector<NamedBitState> visitedTowns;
    std::vector<CurrentScriptState> currentScripts;
    std::vector<RuntimeField> runtimeFields;

    bool gotOldRod = false;
    bool gotGoodRod = false;
    bool gotSuperRod = false;
    bool satisfiedSaffronGuards = false;
    bool gotLapras = false;
    bool everHealedPokemon = false;
    bool gotStarter = false;
    bool defeatedLorelei = false;

    std::string ToString() const;
};

// =========================================================
// Pokédex Summary Model
// =========================================================

class PokedexSummary {
public:
    int ownedCount = 0;
    int seenCount = 0;

    // Dex numbers (1..151) that are owned/seen
    std::vector<int> ownedDexNos;
    std::vector<int> seenDexNos;

    //English names (filled using Gen1SpeciesLookup::PokeDex + NameFromId)
    std::vector<std::string> ownedNames;
    std::vector<std::string> seenNames;
    std::string ToString() const;
    
};

// =========================================================
// Bag Items Models
// =========================================================

class BagItem {
public:
    u8 itemId = 0;
    u8 quantity = 0;

    // English name via Gen1ItemLookup
    std::string itemName;

    // Hex string via Gen1ItemLookup
    std::string itemHex;

    std::string ToString() const;
};

class BagSummary {
public:
    int itemCount = 0;
    std::vector<BagItem> items;

    std::string ToString() const;
};

// =========================================================
// Pokémon Models (Party + PC Boxes)
// =========================================================

class PokemonMove {
public:
    u8 moveId = 0;
    std::string moveName; // optional (add move lookup later)

    u8 ppCurrent = 0;
    u8 ppMax = 0;

    std::string ToString() const;
};

class PokemonStats {
public:
    // Party-only live data (boxes may not have current HP/status stored the same way)
    u16 hpCurrent = 0;
    u16 hpMax = 0;

    u8 status = 0; // raw status byte

    // Core stats
    u16 attack = 0;
    u16 defense = 0;
    u16 speed = 0;
    u16 special = 0;

    std::string ToString() const;
};

class PokemonMon {
public:
    int position = 0; // 1-based index in party/box

    u16 otIdNo = 0;
    u8 speciesId = 0;              // Gen I internal species ID
    std::string speciesName;       // via Gen1SpeciesLookup
    std::string dexNo;             // via Gen1SpeciesLookup::DexfromId (string for JSON convenience)
    

    std::string nickname;          // Gen I text decoded
    std::string otName;            // Gen I text decoded

    u8 level = 0;
    u32 expPoints = 0;
    u8 type1 = 0;
    u8 type2 = 0;
    u8 catchRate = 0;

    // -----------------------------------------------------
    // Gen I hidden values (not shown on most screens)
    // -----------------------------------------------------

    // Stat Exp (Gen I EV-like values). 5 stats, 2 bytes each.
    u16 statExpHP  = 0;
    u16 statExpAtk = 0;
    u16 statExpDef = 0;
    u16 statExpSpd = 0;
    u16 statExpSpc = 0;

    // DVs (Gen I IV-like values). Each DV is 0..15.
    // HP DV is derived from the low bits of Atk/Def/Spd/Spc.
    u8 dvHP  = 0;
    u8 dvAtk = 0;
    u8 dvDef = 0;
    u8 dvSpd = 0;
    u8 dvSpc = 0;

    PokemonStats stats;
    std::vector<PokemonMove> moves; // up to 4

    std::string ToString() const;
};

class PokemonBox {
public:
    int boxNumber = 0;          // 0 = party, 1..12 = PC boxes
    std::string label;          // "Party" or "PC Box N"
    int pokemonCount = 0;

    std::vector<PokemonMon> pokemon;

    std::string ToString() const;
};

class PokemonBoxesExport {
public:
    std::vector<PokemonBox> boxes; // includes party as box 0
    PokemonBox currentBoxCache;
    bool hasCurrentBoxCache = false;

    std::string ToString() const;
};

// =========================================================
// Daycare Model
// =========================================================

class DaycareSummary {
public:
    bool inUse = false;
    PokemonMon pokemon;

    std::string ToString() const;
};

// =========================================================
// Hall of Fame Models (Bank 0)
// =========================================================

class HallOfFamePokemon {
public:
    int partyOrder = 0;
    u8 speciesId = 0;
    u8 level = 0;
    std::string name; // Gen I text decoded Ex: "PIKAPI"
    std::string speciesName; // Ex: "PIKACHU"

    std::string ToString() const;
};

class HallOfFameEntry {
public:
    int entryIndex = 0; // 1..N
    std::vector<HallOfFamePokemon> team; // up to 6

    std::string ToString() const;
};

// =========================================================
// ReadOnlyData (Main Reader Class)
// =========================================================

class ReadOnlyData {
public:
    explicit ReadOnlyData(const SaveBuffer& buffer);

    // --- Core Data ---
    TrainerSummary GetTrainerSummary() const;

    // --- PC Box Statistics ---
    BoxStats GetBoxStats(int boxIndex1to12) const;

    // --- Flags ---
    FlagSummary GetEventFlagSummary() const;
    EventCategorySummary GetEventCategorySummary() const;

    // --- Player / World State ---
    PlayerStateSummary GetPlayerStateSummary() const;
    WorldStateSummary GetWorldStateSummary() const;
    DaycareSummary GetDaycareSummary() const;

    // --- Pokédex ---
    PokedexSummary GetPokedexSummary(bool includeNames = true) const;

    // --- Bag Items ---
    BagSummary GetBagSummary(bool includeNamesAndHex = true) const;
    
    // --- PC Item Box ---
    BagSummary GetPCItemBoxSummary(bool includeNamesAndHex = true) const;

    // --- Pokémon (Party + PC Boxes) ---
    // Party is returned as "box 0" for export uniformity.
    PokemonBox GetPartyAsBox0() const;

    // PC Boxes 1..12 (Banks 2/3).
    PokemonBox GetPCBox(int boxIndex1to12) const;

    // Bank 1 current box cache. This is separate from permanent Boxes 1..12.
    PokemonBox GetCurrentBoxCache() const;

    // Convenience export: Party (box 0) + Boxes 1..12.
    PokemonBoxesExport GetAllBoxesExport() const;

    // --- Hall of Fame (Bank 0) ---
    // Returns an empty list if Hall of Fame record count is 0.
    std::vector<HallOfFameEntry> GetHallOfFame() const;

    // --- Raw Dump ---
    std::string DumpFullSummary() const;

private:
    const SaveBuffer& buffer_;

    // Internal helpers
    int CountBits(u8 byte) const;
};

} // namespace savegenie

#endif /* ReadOnlyData_hpp */
