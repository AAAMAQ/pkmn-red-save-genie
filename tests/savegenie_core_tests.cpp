#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../Pkmn Red Save Genie/HPP Files/ReadOnlyData.hpp"
#include "../Pkmn Red Save Genie/HPP Files/SaveStructure.hpp"
#include "../Pkmn Red Save Genie/HPP Files/WriteOnlyData.hpp"

using namespace savegenie;

static SaveBuffer MakeBlankSave() {
    return SaveBuffer(std::vector<u8>(Gen1Layout::ExpectedSize, 0));
}

static void TestBcdAndChecksum() {
    SaveBuffer save = MakeBlankSave();

    BcdCodec::WriteBcd3(save, Gen1Layout::MoneyOff, 999999);
    assert(BcdCodec::ReadBcd3(save, Gen1Layout::MoneyOff) == 999999);

    BcdCodec::WriteBcd2(save, Gen1Layout::CoinsOff, 4321);
    assert(BcdCodec::ReadBcd2(save, Gen1Layout::CoinsOff) == 4321);

    assert(!Gen1Checksum::ValidateMain(save));
    Gen1Checksum::FixMain(save);
    assert(Gen1Checksum::ValidateMain(save));
}

static void TestTextCodec() {
    SaveBuffer save = MakeBlankSave();
    Gen1TextCodec::EncodeName(save, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen, "MAQ 7");
    assert(Gen1TextCodec::DecodeName(save, Gen1Layout::TrainerNameOff, Gen1Layout::TrainerNameLen) == "MAQ 7");

    Gen1TextCodec::EncodeName(save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen, "Az-9?!");
    assert(Gen1TextCodec::DecodeName(save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen) == "Az-9?!");

    auto& bytes = save.BytesMutable();
    bytes[Gen1Layout::RivalNameOff] = 0xA0;     // a
    bytes[Gen1Layout::RivalNameOff + 1] = 0xB9; // z
    bytes[Gen1Layout::RivalNameOff + 2] = 0xF6; // 0
    bytes[Gen1Layout::RivalNameOff + 3] = 0xFF; // 9
    bytes[Gen1Layout::RivalNameOff + 4] = 0xE3; // -
    bytes[Gen1Layout::RivalNameOff + 5] = 0x50;
    assert(Gen1TextCodec::DecodeName(save, Gen1Layout::RivalNameOff, Gen1Layout::RivalNameLen) == "az09-");
}

static void TestPartyDecodeDvsAndPpMasking() {
    SaveBuffer save = MakeBlankSave();
    auto& bytes = save.BytesMutable();

    bytes[Gen1Layout::PartyCountOff] = 1;
    bytes[Gen1Layout::PartySpeciesOff] = 36; // PIDGEY

    const std::size_t mon = Gen1Layout::PartyStructsOff;
    bytes[mon + 0x00] = 36;
    bytes[mon + 0x01] = 0x00;
    bytes[mon + 0x02] = 0x0F;
    bytes[mon + 0x04] = 0x00;
    bytes[mon + 0x08] = 16; // GUST
    bytes[mon + 0x0C] = 0x10;
    bytes[mon + 0x0D] = 0x01;
    bytes[mon + 0x0E] = 0x00;
    bytes[mon + 0x0F] = 0x00;
    bytes[mon + 0x10] = 0x39;
    bytes[mon + 0x1B] = 0x92; // ATK 9, DEF 2
    bytes[mon + 0x1C] = 0xAE; // SPD 10, SPC 14
    bytes[mon + 0x1D] = 0xE3; // PP Ups in high bits, 35 current PP in low bits
    bytes[mon + 0x21] = 3;
    bytes[mon + 0x22] = 0x00;
    bytes[mon + 0x23] = 0x0F;
    bytes[mon + 0x24] = 0x00;
    bytes[mon + 0x25] = 0x08;
    bytes[mon + 0x26] = 0x00;
    bytes[mon + 0x27] = 0x07;
    bytes[mon + 0x28] = 0x00;
    bytes[mon + 0x29] = 0x08;
    bytes[mon + 0x2A] = 0x00;
    bytes[mon + 0x2B] = 0x07;

    Gen1TextCodec::EncodeName(save, Gen1Layout::PartyOTNamesOff, Gen1Layout::Gen1NameLen, "MARIO");
    Gen1TextCodec::EncodeName(save, Gen1Layout::PartyNicknamesOff, Gen1Layout::Gen1NameLen, "PEGGY");

    ReadOnlyData reader(save);
    const PokemonBox party = reader.GetPartyAsBox0();
    assert(party.pokemonCount == 1);
    assert(party.pokemon.size() == 1);
    const PokemonMon& pidgey = party.pokemon[0];
    assert(pidgey.speciesId == 36);
    assert(pidgey.nickname == "PEGGY");
    assert(pidgey.otName == "MARIO");
    assert(pidgey.level == 3);
    assert(pidgey.expPoints == 57);
    assert(pidgey.dvHP == 8);
    assert(pidgey.moves.size() == 4);
    assert(pidgey.moves[0].ppCurrent == 35);
}

static void TestSafeEditorSetters() {
    SaveBuffer save = MakeBlankSave();
    auto& bytes = save.BytesMutable();
    bytes[Gen1Layout::BagItemsCountOff] = 1;
    bytes[Gen1Layout::BagItemsPairsOff] = 0x04;
    bytes[Gen1Layout::BagItemsPairsOff + 1] = 1;

    WriteOnlyData editor(save);
    assert(editor.SetMoney(123456).Ok());
    assert(editor.SetCoins(9876).Ok());
    assert(editor.SetTrainerName("MAQ").Ok());
    assert(editor.SetRivalName("KILLUA").Ok());
    assert(editor.SetBadges(0x81).Ok());
    assert(bytes[Gen1Layout::BadgesOff] == 0x81);
    assert(bytes[Gen1Layout::BadgesMirrorOff] == 0x81);
    assert(editor.SetLocation(1, 2, 3).Ok() == false);
    assert(editor.SetItemQuantity(ItemListKind::Bag, 0x04, 9).Ok());
    assert(editor.SetTrainerName("BAD!").Ok() == false);
    assert(editor.FixChecksums().Ok());

    ReadOnlyData reader(save);
    const TrainerSummary trainer = reader.GetTrainerSummary();
    assert(trainer.money == 123456);
    assert(trainer.coins == 9876);
    assert(trainer.trainerName == "MAQ");
    assert(trainer.rivalName == "KILLUA");
    assert(reader.GetBagSummary(false).items[0].quantity == 9);
    assert(Gen1Checksum::ValidateMain(save));
}

static void TestWorldPlayerEventAndDaycareSummaries() {
    SaveBuffer save = MakeBlankSave();
    auto& bytes = save.BytesMutable();

    bytes[Gen1Layout::OptionsOff] = 0xA5;
    bytes[Gen1Layout::LetterDelayOff] = 0x03;
    bytes[Gen1Layout::ContrastOff] = 0x06;
    bytes[Gen1Layout::YBlockCoordOff] = 11;
    bytes[Gen1Layout::XBlockCoordOff] = 22;
    bytes[Gen1Layout::SpecialWarpYOff] = 2;
    bytes[Gen1Layout::SpecialWarpXOff] = 4;
    bytes[Gen1Layout::PlayerMoveDirOff] = 0x01;
    bytes[Gen1Layout::PlayerLastStopDirOff] = 0x02;
    bytes[Gen1Layout::PlayerCurDirOff] = 0x08;
    bytes[Gen1Layout::WalkBikeSurfOff] = 0x02;
    bytes[Gen1Layout::PlayerJumpingYScreenOff] = 7;
    bytes[Gen1Layout::SafariStepsOff] = 0x01;
    bytes[Gen1Layout::SafariStepsOff + 1] = 0x23;
    bytes[Gen1Layout::SafariGameOverOff] = 1;
    bytes[Gen1Layout::SafariBallCountOff] = 30;

    save.SetBit(Gen1Layout::WorldFlags1Off, 0, true); // strength
    save.SetBit(Gen1Layout::WorldFlags1Off, 1, true); // surf
    save.SetBit(Gen1Layout::WorldFlags1Off, 3, true); // old rod
    save.SetBit(Gen1Layout::WorldFlags1Off, 6, true); // Saffron guards
    save.SetBit(Gen1Layout::WorldFlags1Off, 7, true); // card key
    save.SetBit(Gen1Layout::BattleFlagsOff, 6, true);
    save.SetBit(Gen1Layout::WorldFlags2Off, 0, true); // Lapras
    save.SetBit(Gen1Layout::WorldFlags2Off, 3, true); // starter
    save.SetBit(Gen1Layout::FlyFlagsOff, 7, true);
    save.SetBit(Gen1Layout::EliteFlagsOff, 1, true);
    save.SetBit(Gen1Layout::DoorWarpFlagsOff, 2, true);
    save.SetBit(Gen1Layout::TextFlagsOff, 6, true);
    save.SetBit(Gen1Layout::PlaytimeFlagsOff, 0, true);

    save.SetBit(Gen1Layout::MissableObjectsOff, 0, true);
    save.SetBit(Gen1Layout::MissableObjectsOff + 28, 3, true); // used bit 227
    save.SetBit(Gen1Layout::HiddenItemsOff, 5, true);
    save.SetBit(Gen1Layout::HiddenCoinsOff + 1, 3, true); // used bit 11
    save.SetBit(Gen1Layout::VisitedTownsOff, 0, true);
    save.SetBit(Gen1Layout::VisitedTownsOff + 1, 2, true); // used bit 10
    bytes[Gen1Layout::CurrentScriptsOff + 3] = 2;

    save.SetBit(Gen1Layout::EventFlagsOff + (119 / 8), static_cast<u8>(119 % 8), true);  // Brock
    save.SetBit(Gen1Layout::EventFlagsOff + (191 / 8), static_cast<u8>(191 % 8), true);  // Misty
    save.SetBit(Gen1Layout::EventFlagsOff + (37 / 8), static_cast<u8>(37 % 8), true);    // Pokedex
    save.SetBit(Gen1Layout::EventFlagsOff + (1378 / 8), static_cast<u8>(1378 % 8), true); // Viridian Forest trainer #1
    save.SetBit(Gen1Layout::EventFlagsOff + (2522 / 8), static_cast<u8>(2522 % 8), true); // Articuno
    bytes[Gen1Layout::BadgesOff] = 0x01; // Brock badge only; Misty creates mismatch.

    bytes[Gen1Layout::DaycareInUseOff] = 1;
    bytes[Gen1Layout::DaycareBoxMonOff + 0x00] = 36; // PIDGEY
    bytes[Gen1Layout::DaycareBoxMonOff + 0x0C] = 0x10;
    bytes[Gen1Layout::DaycareBoxMonOff + 0x0D] = 0x01;
    bytes[Gen1Layout::DaycareBoxMonOff + 0x21] = 5;
    Gen1TextCodec::EncodeName(save, Gen1Layout::DaycareNicknameOff, Gen1Layout::Gen1NameLen, "BIRD");
    Gen1TextCodec::EncodeName(save, Gen1Layout::DaycareOTNameOff, Gen1Layout::Gen1NameLen, "MAQ");

    ReadOnlyData reader(save);
    const PlayerStateSummary player = reader.GetPlayerStateSummary();
    assert(player.optionsByte == 0xA5);
    assert(player.xBlockCoord == 22);
    assert(player.yBlockCoord == 11);
    assert(player.MovementModeName() == "Surfing");
    assert(player.safariSteps == 0x0123);
    assert(player.safariGameOver);
    assert(player.strengthOutsideBattle);
    assert(player.surfingAllowed);
    assert(player.flyOutOfBattle);
    assert(player.noLetterDelay);
    assert(player.countPlaytime);

    const WorldStateSummary world = reader.GetWorldStateSummary();
    assert(world.missableObjectsChecked == Gen1Layout::MissableObjectsUsedBits);
    assert(world.missableObjectsSet == 2);
    assert(world.hiddenItemsCollected == 1);
    assert(world.hiddenCoinsCollected == 1);
    assert(world.visitedTownsSet == 2);
    assert(world.currentScriptsNonZero == 1);
    assert(world.gotOldRod);
    assert(world.satisfiedSaffronGuards);
    assert(world.gotLapras);
    assert(world.gotStarter);
    assert(world.defeatedLorelei);

    const EventCategorySummary events = reader.GetEventCategorySummary();
    assert(events.defeatedTrainerFlagsSet == 3);
    assert(events.trainerFlagsKnown > events.trainerFlagsComplete);
    assert(events.trainerFlagsComplete == 3);
    assert(events.storyFlagsKnown > events.storyFlagsComplete);
    assert(events.staticEncounterFlagsKnown > events.staticEncounterFlagsComplete);
    assert(events.staticEncounterFlagsComplete == 1);
    assert(events.gymLeaderFlagsSet == 2);
    assert(events.majorStoryMilestonesSet == 1);
    assert(events.legendaryFlagsSet == 1);
    assert(events.gymStoryMismatchCount == 1);

    bool foundViridianTrainer = false;
    for (const auto& flag : events.trainerFlags) {
        if (flag.eventName == "EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_0") {
            foundViridianTrainer = true;
            assert(flag.label == "Trainer #1, Viridian Forest");
            assert(flag.location == "Viridian Forest");
            assert(flag.trainerNumber == 1);
            assert(flag.isComplete);
        }
    }
    assert(foundViridianTrainer);

    const DaycareSummary daycare = reader.GetDaycareSummary();
    assert(daycare.inUse);
    assert(daycare.pokemon.speciesId == 36);
    assert(daycare.pokemon.speciesName == "PIDGEY");
    assert(daycare.pokemon.nickname == "BIRD");
    assert(daycare.pokemon.otName == "MAQ");
    assert(daycare.pokemon.otIdNo == 4097);
    assert(daycare.pokemon.level == 5);
}

int main() {
    TestBcdAndChecksum();
    TestTextCodec();
    TestPartyDecodeDvsAndPpMasking();
    TestSafeEditorSetters();
    TestWorldPlayerEventAndDaycareSummaries();
    std::cout << "savegenie_core_tests: PASS\n";
    return 0;
}
