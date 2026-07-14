# Research Findings Log

Project: `pkmn-red-save-genie`

Creator: MAQ / BiG MAQ Studios

Last updated: 2026-06-20

## Finding R-001 - Gen I Internal Species IDs Are Not Pokedex Numbers

Date: 2026-06-19

Status: Confirmed

Source: `tmp/pdfs/List of Pokemon by index number in Generation I - Bulbapedia...txt`; `Pkmn Red Save Genie/CPP Files/SaveStructure.cpp`; `Gen1SpeciesLookup`

Evidence: Bulbapedia's Gen I index reference states that Pokemon internal IDs are not National Pokedex order. The current code implements `SpeciesName`, `SpeciesNo`, `SpeciesHex`, and `PokeDex` mapping tables.

Previous assumption: Species ID could be treated as Pokedex number.

Finding: Gen I internal species IDs require lookup and Pokedex translation.

Implementation impact: Exports include both `speciesId` and `dexNo`.

Validation: Party output maps internal ID 111 to POLIWRATH and Dex 62.

Remaining questions: How should glitch species and MissingNo-like entries be handled in future conversion?

## Finding R-002 - Main Gen I Checksum Range and Algorithm

Date: 2026-06-19

Status: Confirmed

Source: `Pkmn Red Save Genie/CPP Files/SaveStructure.cpp`; `tmp/pdfs/Save data structure (Generation I)...txt`; `pret/pokered` external analysis

Evidence: `Gen1Checksum::ComputeMain()` sums bytes `0x2598..0x3522` inclusive and returns bitwise inverted low byte. The checksum byte is `0x3523`.

Previous assumption: Checksum might be a normal sum or broader bank checksum.

Finding: Main checksum is the complement of the low 8 bits of the covered sum.

Implementation impact: All future writes inside the covered range must call `Gen1Checksum::FixMain()`.

Validation: Generated `SaveGenieSummary.txt` reports `Main Checksum: VALID`.

Remaining questions: None for main checksum; bank/per-box checksum write repair needs future edit tests.

## Finding R-003 - Gen I Text Is Not ASCII

Date: 2026-06-19

Status: Confirmed

Source: `Gen1TextCodec` in `SaveStructure.cpp`; generated outputs; project brain text notes

Evidence: `Gen1TextCodec::ByteToAscii()` maps `0x80..0x99` to `A..Z`, `0xA0..0xB9` to `a..z`, `0xF6..0xFF` to digits, common punctuation bytes such as `0xE3` to `-`, `0x7F` to space, and `0x50` to terminator. The corrected mapping was cross-checked against Junebug v1 `src/assets/data/text.json`.

Previous assumption: Names could be read as ASCII strings.

Finding: Gen I text needs explicit decode/encode logic.

Implementation impact: Trainer names, rival names, OT names, nicknames, and Hall of Fame names use `DecodeName()`.

Validation: The PEGGY/MARIO test case decodes correctly, and `tests/savegenie_core_tests.cpp` now checks lowercase, digits, punctuation, and raw byte decoding.

Remaining questions: Which additional glyphs should be supported, especially punctuation, lowercase, and modded/hacked bytes?

## Finding R-004 - Party Pokemon Store Live Stats

Date: 2026-06-19

Status: Confirmed by current source code and project validation notes

Source: `ReadOnlyData.cpp`; project brain validated party anchors

Evidence: `DecodePartyMonStruct()` reads current HP, status, max HP, attack, defense, speed, and special from the party struct.

Previous assumption: Party and box structures could be handled the same way.

Finding: Party Pokemon contain live battle stats; boxed Pokemon do not store the same party live stats.

Implementation impact: Party `stats` are populated, while boxed Pokemon visible stats remain zero/default unless computed later.

Validation: Screenshot-validated anchors for POLIWRATH, ALAKAZAM, ARTICUNO, MEWTWO, GOLEM, and PEGGY.

Remaining questions: Whether future exports should compute boxed stats from species/base stats, level, DVs, and Stat Exp.

## Finding R-005 - PP Bytes Need Masking

Date: 2026-06-19

Status: Confirmed

Source: Project brain; `ReadOnlyData.cpp`

Evidence: `DecodePartyMonStruct()` and `DecodeBoxMonStruct()` compute `ppCurrent = rawPP & 0x3F`.

Previous assumption: PP byte could be read directly.

Finding: Low 6 bits are current PP; high 2 bits represent PP Ups used.

Implementation impact: Current output uses masked PP. `ppMax` remains a placeholder equal to current PP.

Validation: Party moves produce expected current PP values.

Remaining questions: Add base PP table and PP Up max calculation only when requested.

## Finding R-006 - Current Box Cache Is Separate From Permanent Boxes

Date: 2026-06-19

Status: Confirmed by external references and current constants

Source: `Gen1Layout::CurrentBoxCacheOff`; Bulbapedia Gen I PDF text; `pret/pokered` source map; `pokered-save-editor`; Game Tools Collection

Evidence: Current box cache is at `0x30C0`, length `0x0462`, and has the same internal layout as a PC box block.

Previous assumption: Permanent boxes 1-12 were the only relevant PC storage.

Finding: The current in-game box may have a fresher Bank 1 cached copy.

Implementation impact: Add `ReadOnlyData::GetCurrentBoxCache()` as a low-risk reader extension.

Validation: Not yet implemented in current project.

Remaining questions: When exporting, should the cache be separate as `currentBoxCache` or appended as a pseudo-box?

## Finding R-007 - Trailing Save Bytes Exist in Real Outputs

Date: 2026-06-19

Status: Verified against generated output

Source: `SaveGenieSummary.txt`; `PokemonSummary.json`

Evidence: Generated output shows save size `0x802c` / 32812 bytes while main checksum is valid.

Previous assumption: All saves must be exactly `0x8000`.

Finding: Emulator/dump files may include trailing bytes.

Implementation impact: Tool currently warns. Future policy should parse first `0x8000` bytes and warn that trailing data is ignored.

Validation: Current generated output remains readable and checksum-valid.

Remaining questions: Which emulators append which metadata and should it ever be preserved?

## Finding R-008 - PC Box Decode Needs Verification

Date: 2026-06-19

Status: Resolved on 2026-07-14; superseded by Finding R-020

Source: `PokemonBoxes.json`; `ReadOnlyData.cpp`

Evidence: Generated `PokemonBoxes.json` includes some boxed Pokemon levels above 100. The source code comment says many tools treat boxed level as `+0x03`, but `DecodeBoxMonStruct()` reads `monOff + 0x21` after requiring a `0x21`-byte struct.

Previous assumption: PC box decode was fully plausible based on successful export generation.

Finding: The concern was valid. The old decoder read one byte past the `0x21`-byte record. Correct decoding reads stored level at `+0x03` and stored current HP at `+0x01..+0x02`.

Implementation impact: The parser and semantic export were corrected, with boundary and post-withdrawal regression tests.

Validation: Source bytes and equivalent emulator withdrawals confirmed `201/201` for the Box 1 Dugtrio; generated zero HP was traced to the old parser output.

Remaining questions: Broader corpus validation remains useful, but the offset question is resolved.

## Finding R-009 - Text Exports Are Debug/Audit Artifacts, Not Conversion Source of Truth

Date: 2026-06-19

Status: Confirmed by supplied planning discussion

Source: `tmp/pdfs/Pokemon Red to Fire Red detailed conversation.txt`

Evidence: Later planning corrected the early idea of using text as the conversion layer. Text is useful for inspection, but structured models must drive save writing.

Previous assumption: Red save to text and FireRed text rewrite could simplify conversion.

Finding: Text does not remove binary complexity; it is best used for audit, debugging, and reproducible reports.

Implementation impact: Future converter should use `RedSaveModel`, `FireRedSaveModel`, conversion policies, and writers, while emitting text/JSON reports.

Validation: Current project already has text and JSON export for diagnostics.

Remaining questions: Should there later be a strict JSON import/edit format?

## Finding R-010 - Pokemon Transfer Is Not Whole-Save Conversion

Date: 2026-06-19

Status: Confirmed by external project analysis

Source: `docs/external_projects_deep_analysis.md`; PCCS and Poke Transporter GB analysis

Evidence: Community projects solve Pokemon conversion/transport pieces, but none analyzed was found to reconstruct a full FireRed save from a Red save model.

Previous assumption: Pokemon-level transfer might imply full save conversion is solved.

Finding: Pokemon conversion is one subsystem. Whole-save conversion also needs trainer, Pokedex, inventory, flags, location, FireRed sections, checksums, and consistency rules.

Implementation impact: Future architecture needs separate modules for Pokemon conversion, FireRed save writing, progress mapping, location mapping, and reports.

Validation: External analysis complete as of 2026-06-19.

Remaining questions: Which external code can be reused legally and technically?

## Finding R-011 - FireRed Save Writing Must Start With Sections, Not Fields

Date: 2026-06-19

Status: Confirmed by external reference and supplied planning discussion

Source: Bulbapedia Gen III PDF text; `docs/external_projects_deep_analysis.md`; `pret/pokefirered` source map

Evidence: FireRed saves use two save blocks, 14 sections per block, section IDs, signatures, checksums, and save indices.

Previous assumption: FireRed could be edited like Gen I fixed offsets from file start.

Finding: A FireRed writer must parse active save slot and section map before editing fields.

Implementation impact: Future modules should begin with `Gen3SaveContainer`, `Gen3SectionReader`, and `Gen3SectionWriter`.

Validation: Not implemented.

Remaining questions: Exact first semantic write should be a low-risk SaveBlock2 field after unchanged round-trip validation.

## Finding R-012 - Sevii Islands Are a Conversion Consistency Risk

Date: 2026-06-19

Status: Confirmed by supplied expert discussion and external FireRed source map

Source: Inspiration file Zayaldrie advice; `docs/external_projects_deep_analysis.md`; `pret/pokefirered` source map

Evidence: FRLG includes Sevii Islands progression before/around Cinnabar and postgame. Flags, vars, National Dex state, and travel unlocks can conflict.

Previous assumption: Badge/League state could be copied without broader story implications.

Finding: Converted postgame state can be inconsistent unless Sevii/National Dex/flags/vars are coherent.

Implementation impact: First converter should use conservative or hybrid story policy.

Validation: Not implemented.

Remaining questions: What exact FireRed flags/vars define a safe post-League or pre-postgame state?

## Finding R-013 - Junebug World-State Sections Are Useful Coverage Guides

Date: 2026-06-20

Status: Confirmed from external source code and current implementation

Source: `junebug12851/pokered-save-editor`; `junebug12851/pokered-save-editor-2`; `ReadOnlyData::GetWorldStateSummary()`; `ReadOnlyData::GetPlayerStateSummary()`

Evidence: Junebug v1/v2 model world events, scripts, missables, hidden items, hidden coins, visited towns, area/player state, and Daycare in separate save sections. Save Genie now mirrors several of those categories as read-only diagnostics while keeping event names anchored to `pret/pokered`.

Previous assumption: Save Genie only had raw event-flag counts for story/world state.

Finding: The Gen I save can be usefully summarized into world-state categories before full story editing exists.

Implementation impact: `PokemonSummary.json` and `SaveGenieSummary.txt` now include compact world/player/daycare/event-category data without exposing new editors.

Validation: Synthetic tests set individual bits/bytes and verify decoded counts, booleans, event categories, and Daycare data.

Remaining questions: Should Save Genie import Junebug's full 97-entry script metadata table, or should script names come directly from `pret/pokered` symbols?

## Finding R-014 - Daycare Uses A Boxed-Pokemon-Style Record

Date: 2026-06-20

Status: Confirmed from external source code and current implementation

Source: Junebug v1 `Daycare.ts`; Junebug v2 `daycare.cpp`; `ReadOnlyData::GetDaycareSummary()`

Evidence: Both Junebug editors read the Daycare in-use byte at `0x2CF4`, nickname at `0x2CF5`, OT name at `0x2D00`, and the deposited Pokemon record at `0x2D0B`.

Previous assumption: Daycare was part of miscellaneous main-save data and not separately decoded.

Finding: Daycare can be safely decoded read-only using the existing boxed-Pokemon decoder pattern.

Implementation impact: Save Genie now exports Daycare in the terminal/text summary and compact JSON.

Validation: Synthetic test creates a deposited Pidgey with nickname/OT/OT ID and validates the decoded output.

Remaining questions: Validate Daycare behavior against a real save with a deposited Pokemon before claiming screenshot-level verification.

## Finding R-015 - pret Event Constants Can Produce Complete Trainer Progress Rows

Date: 2026-06-20

Status: Confirmed from external source code and current implementation

Source: `pret/pokered` commit `d70d99ffbd329473d96eaaf19fd97c86d2220b7f`, `constants/event_constants.asm`, `macros/scripts/events.asm`, `scripts/ViridianForest.asm`; `ReadOnlyData::GetEventFlagSummary()`; `ReadOnlyData::GetEventCategorySummary()`

Evidence: `constants/event_constants.asm` defines the numeric event-bit order using `const_def`, `const`, `const_next`, and `const_skip`. Script files use trainer macros such as `trainer EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_0`, confirming those event bits represent defeated trainer state. The local reader now stores all known named event labels as true/false values and derives readable trainer rows such as `Trainer #1, Viridian Forest: true`.

Previous assumption: Event output was limited to raw set-counts, raw set indices, and a small set of named flags when they were true.

Finding: Save Genie can safely decode named event state read-only by mapping the existing event bitset through the authoritative pret event constants. `EVENT_BEAT_*_TRAINER_N` symbols can be shown as numbered trainer-completion rows, while one-off battles such as Articuno, Snorlax, Marowak, and Power Plant Voltorb entries should remain separate from trainer progress.

Implementation impact: `SaveGenieSummary.txt` now includes complete true/false known event labels, trainer-progress rows, story/world categories, static battle rows, and gym/badge consistency checks. `PokemonSummary.json` exports the same categories in machine-readable form. No event editing was added.

Validation: Synthetic tests set Brock, Misty, Pokédex, Articuno, and `EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_0`, then assert the decoded trainer row `Trainer #1, Viridian Forest` is complete.

Remaining questions: Current-map object/warp/sign cache rows and enemy/link union data still need deeper research before they can be considered fully human-readable.

## Finding R-016 - World-State Metadata Can Be Named Without Expanding Editor Scope

Date: 2026-06-20

Status: Confirmed from external source code and current implementation

Source: `pret/pokered` commit `d70d99ffbd329473d96eaaf19fd97c86d2220b7f`, `ram/wram.asm`, `data/events/hidden_item_coords.asm`, `data/events/hidden_coins.asm`; `junebug12851/pokered-save-editor-2`, `projects/db/assets/data/scripts.json`, `missables.json`, `hiddenItems.json`, `hiddenCoins.json`; `ReadOnlyData::GetWorldStateSummary()`

Evidence: `pret/pokered` identifies the Bank 1 world-state symbols for current scripts, toggleable/missable object flags, hidden item flags, hidden coin flags, visited-town flags, map header/runtime fields, status bytes, completed trades, warp bookkeeping, and current-map script state. Junebug/Twilight's v2 DB provides curated names/counts for the 97 script values, 228 missable objects, 54 hidden items, 12 hidden coins, and 11 Fly destination flags. Save Genie now exports those categories as read-only English records while preserving unknown/padding/runtime bytes.

Previous assumption: Save Genie could only show counts and first set indices for these categories until a larger event-system refactor.

Finding: Complete named read-only output for used current scripts, missables, hidden items, hidden coins, and visited towns can be added conservatively without allowing writes or pretending every adjacent runtime byte is semantically understood.

Implementation impact: `WorldStateSummary` now includes `currentScripts`, `missableObjects`, `hiddenItems`, `hiddenCoins`, `visitedTowns`, and `runtimeFields`. `PokemonSummary.json` and `SaveGenieSummary.txt` include these richer English records. The Safe Editor scope remains unchanged.

Validation: Synthetic tests assert named entries for Prof. Oak, Articuno, S.S. Anne Kitchen hidden item, Game Corner hidden coins, Pallet Town/Saffron City Fly flags, and the Blues House current-script value.

Remaining questions: Current-map warp entries, sign rows, cached sprite rows, enemy/link union data, and the post-checksum `0x3524-0x3FFF` range still need research before any stronger completeness claim.

## Finding R-017 - Physical Image Must Remain The Reconstruction Authority

Date: 2026-06-22

Status: Implemented and verified for the Red-side `.red.json` milestone

Source: `RedMasterJson`; `tests/red_master_json_tests.cpp`; `docs/RED_MASTER_JSON_CONVENTION.md`; `docs/RED_MASTER_JSON_ROUND_TRIP.md`

Evidence: Synthetic standard, trailing-byte, and semantic fixtures reconstruct with zero byte differences. `physicalImage.standardSramHex` and `physicalImage.trailingDataHex` preserve the source bytes directly.

Previous assumption: A future master export might eventually be rebuilt from decoded semantic objects.

Finding: No-edit reconstruction must use the physical byte image, not decoded semantic fields. Decoded fields are inspection and conversion-source data, not the archival authority.

Implementation impact: `.red.json` contains both `physicalImage` and `decoded`. Semantic expansion cannot corrupt no-edit reconstruction. Unknown, runtime, scratch, and trailing bytes remain preserved even when not fully interpreted.

Validation: `red_master_json_tests` verifies byte-identical reconstruction and semantic expansion guardrails.

Remaining questions: Future edited reconstruction must define explicit semantic edit application rules before altering raw bytes.

## Finding R-018 - Red-Side Conversion Readiness Is Complete Before FireRed Work Begins

Date: 2026-06-22

Status: Implemented as Red-side source foundation

Source: `docs/RED_MASTER_JSON_COMPLETION_MILESTONE.md`; `docs/PROJECT_PUBLIC_RESEARCH_RELEASE.md`; `docs/CONVERSION_MODEL.md`; `docs/RED_TO_FIRERED_MAPPING.md`

Evidence: `.red.json` exports decoded gameplay data and a top-level `conversionModel`. Concepts are classified as direct transfer, semantic translation, Red-only preservation, or unsupported/policy-required.

Previous assumption: More Red-side expansion might be needed before beginning FireRed research.

Finding: The Red source side is complete enough for the converter foundation. Further work should focus on FireRed `.fred.json`, FireRed reader/writer, Gen III Pokémon serialization, and emulator validation rather than redesigning `.red.json`.

Implementation impact: Red Save Genie documentation now marks `.red.json` stable unless a real bug is found. FireRed tasks are explicitly future-phase items.

Validation: Repository tests verify Red round-trip behavior. Human-reported playtesting/emulator/bug-testing results are accepted for the documentation release pass unless contradicted by repository evidence.

Remaining questions: FireRed section handling, Pokémon encryption, target mappings, and converted-save validation remain open.

## Finding R-019 - Display Text Alone Is Not A Lossless Semantic Value

Date: 2026-07-14

Status: Confirmed by source bytes, emulator display, and corrective round-trip tests

Evidence: An OT name contained byte `0xF2`, rendered by the game as `.`, but the former partial codec decoded it through the unknown fallback as `?`. Re-encoding changed it to a different byte.

Finding: Visually equivalent Unicode text can hide distinct Gen I bytes, while an unknown fallback can destroy a valid glyph. Save Genie must expose both display text and a lossless token representation.

Implementation impact: `DecodeNameLossless()` emits `<DOT>`, `<PERIOD>`, named glyph tokens, and `<0xHH>` escapes. Corrective tests prove byte-aware round trips.

## Finding R-020 - Boxed Records Store HP And Level Inside The 0x21-Byte Record

Date: 2026-07-14

Status: Confirmed by binary comparison and post-withdrawal emulator evidence

Evidence: A valid Box 1 Dugtrio record begins with species, HP `0x00C9`, and level `0x64`. The former decoder read level at `+0x21`, one byte beyond the record, and emitted zero HP. The generated Pokemon subsequently withdrew as `0/201`; the source withdrew as `201/201`.

Finding: Current HP is stored at `+0x01..+0x02`, level at `+0x03`, status at `+0x04`, types at `+0x05..+0x06`, and catch rate at `+0x07`.

Implementation impact: Save Genie now exports these fields from their verified offsets. Tests guard the record boundary and withdrawal-relevant values.

## Finding R-021 - The Bank 1 Current Box Can Be Authoritative Player-Visible State

Date: 2026-07-14

Status: Confirmed by source binary and emulator behavior

Evidence: The completed source save has selected byte `0x8B`, empty permanent Box 12, and 20 Pokemon in the Bank 1 current working box. Bill's PC displays those 20 Pokemon.

Finding: Current-box and permanent selected-box divergence is not automatically corruption. Consumers must preserve and compare both structures rather than deriving the working copy unconditionally.

Implementation impact: `.red.json` identifies the selected number, high-bit history state, permanent/cache comparison, and full working-box contents separately.

## Finding R-022 - Hall Of Fame Species Use Internal IDs In Fixed Slots

Date: 2026-07-14

Status: Confirmed by raw records and emulator-visible failure

Evidence: Charizard is stored as internal species ID `0xB4`. Filtering IDs numerically above 151 removed Charizard from each semantic team, compressed following slots, and produced missing members or `?? BIRD` in a generated save.

Finding: Each Hall of Fame team is six fixed `0x10`-byte slots within a `0x60`-byte record. Internal species validity must use the Gen I mapping table, not a National Dex numeric range.

Implementation impact: Save Genie preserves `partyOrder`, validates internal IDs, and exports all populated slots without vector compression.

## Finding R-023 - Project Licensing Uses Standard MIT Terms

Date: 2026-07-14

Status: Owner-approved and documented

Evidence: Repository-root `LICENSE`; generator repository `LICENSE`; public README license sections

Finding: Save Genie and Save Generator use the standard MIT License under the project identity `MAQ / BiG MAQ Studios`. A shared stewardship note asks users to favor education, research, archival preservation, and retro-development rather than merely repackaging the work for sale.

Implementation impact: The stewardship wording is explicitly non-binding and does not alter any MIT permission. Third-party materials retain their own licenses and attribution requirements.
