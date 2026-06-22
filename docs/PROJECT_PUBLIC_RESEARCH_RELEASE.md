# Project Public Research Release

Date: 2026-06-22

Status: Pokémon Red Save Genie is complete for its intended Red-side research, verification, archival, and converter-source role.

## Final Thesis

Pkmn Red Save Genie demonstrates that a Pokémon Red save can be treated as both:

- a lossless archival binary object that can be reconstructed byte-for-byte; and
- a structured semantic source model that exposes the transferable gameplay state needed for future Red-to-FireRed conversion research.

The project is not complete as a Red-to-FireRed converter. It is complete as the Pokémon Red source foundation that such a converter can safely build on.

## Evidence Categories

The documentation distinguishes these evidence types:

- Repository evidence: source code, tests, schema, and committed docs.
- Automated verification: synthetic unit/round-trip tests and Xcode builds recorded during implementation.
- Human verification: reported playtesting, emulator checks, save-diff review, and bug-testing results supplied by MAQ and accepted for this documentation pass unless contradicted by repository evidence.
- Informed interpretation: conclusions supported by `pret/pokered`, Junebug/Twilight editor research, project experiments, and internal consistency.
- Future work: FireRed, Gen III, and conversion tasks not implemented in this Red-side release.

## What Was Built

Save Genie now includes:

- Gen I save loading, validation, backup-safe workflow, and checksum reporting.
- Read-only decoding for trainer/rival identity, trainer ID, money, coins, badges, location, playtime, inventory, Pokédex, Hall of Fame, party, PC boxes, current box cache, Daycare, event flags, trainer flags, story/static categories, scripts, missable objects, hidden items, hidden coins, visited towns, and selected world/runtime fields.
- A conservative Safe Editor MVP for money, coins, trainer name, rival name, badges, and quantities of existing bag/PC item entries.
- Legacy/test exports through `SaveGenieSummary.txt`, `PokemonBoxes.json`, and `PokemonSummary.json`.
- Canonical `.red.json` export/import/reconstruction through `RedMasterJson`.
- Full raw-byte preservation of the standard `0x0000-0x7FFF` SRAM range plus trailing bytes.
- Coverage records with zero uncovered bytes and zero accidental primary overlaps.
- A `decoded` hierarchy and `conversionModel` suitable as the Red source layer for a future converter.

## Why `.red.json` Is Complete For The Red Side

The Red master JSON milestone is considered complete because:

- no-edit reconstruction uses `physicalImage` and produces byte-identical saves in the test harness;
- source hashes, standard SRAM hashes, and trailing-data hashes are recorded;
- unknown/runtime/scratch bytes are preserved instead of discarded or reinterpreted;
- the major transferable gameplay concepts are decoded into a stable semantic hierarchy;
- conversion classifications separate direct transfer, semantic translation, Red-only preservation, and policy-required data;
- future FireRed target fields are intentionally documented as policy/output responsibilities rather than Red decoder gaps.

The schema remains version `0.1.0` because it is a draft public-development schema, not because the Red-side milestone is incomplete.

## Physical Versus Semantic Authority

`physicalImage` is the archival and no-edit reconstruction authority. It must be used to rebuild an unchanged save exactly.

`decoded` and `conversionModel` are the semantic and conversion-source authority. They explain the gameplay meaning of the save and prepare data for later mapping into a layout-independent conversion model.

This separation prevents decoded fields, inferred meanings, or future conversion policies from accidentally corrupting the original save image.

## Coverage Interpretation

The project has complete byte preservation and classification, not complete English naming for every byte.

Complete:

- every byte in `0x0000-0x7FFF` is covered by a primary range;
- trailing data after `0x7FFF` is preserved;
- major gameplay concepts are decoded.

Intentionally not overclaimed:

- runtime caches;
- scratch buffers;
- unused and padding ranges;
- unknown Bank 1 and bank-tail regions;
- temporary map-local state without stable user-facing meaning.

Those bytes remain useful because they are preserved losslessly and documented honestly.

## Validation Assumptions

For this release-prep pass, MAQ's reported human verification, playtesting, emulator checks, and bug-testing results are accepted as correct unless repository evidence contradicts them.

Repository evidence currently supports:

- `red_master_json_tests`: PASS during milestone completion.
- `savegenie_core_tests`: PASS during milestone completion.
- Xcode build: PASS during milestone completion.
- `.red.json` standard, trailing-byte, and semantic synthetic round trips: zero byte differences.
- Draft JSON Schema validation: PASS during milestone completion.

Future public release work may add a broader real-save validation corpus, but the absence of that corpus should not reopen the completed `.red.json` design by itself.

## External Sources And Credits

Important sources:

- `pret/pokered`: event constants, SRAM/WRAM symbols, scripts, game behavior, and map/story research.
- `pret/pokefirered`: future FireRed section, flag, variable, map, and save research.
- Junebug/Twilight `pokered-save-editor` and `pokered-save-editor-2`: Gen I save editor architecture, current box cache behavior, world-state coverage cross-checks, text/table references, and test strategy inspiration.
- Bulbapedia and Glitch City Wiki: save structure, species, item, and map index references.
- Gears of Progress and related community transfer work: future Pokémon-level conversion policy context.

The project uses its own C++ implementation. External repositories are credited as research and validation references unless a future change explicitly adapts code under a compatible license.

## Intentionally Deferred

Deferred from the completed Red-side release:

- `.fred.json`
- FireRed reader/writer
- FireRed save-slot/section/checksum/security-key handling
- Gen III Pokémon encryption and substructure permutation
- FireRed target item/move/species/flag/variable/map IDs
- final DV-to-IV, StatExp-to-EV, PID, nature, ability, friendship, met-location, and Poké Ball generation policies
- actual Red-to-FireRed save generation
- emulator validation of converted FireRed saves

These are future converter phases, not missing Red Save Genie features.

## Authoritative Documentation Map

- `README.md`: public overview, safety, usage, and phase boundary.
- `docs/RED_MASTER_JSON_COMPLETION_MILESTONE.md`: why `.red.json` is complete for the Red side.
- `docs/RED_MASTER_JSON_CONVENTION.md`: canonical `.red.json` rules.
- `docs/RED_MASTER_JSON_ROUND_TRIP.md`: reconstruction and verification workflow.
- `docs/RED_SAVE_COVERAGE.md`: master JSON byte coverage rules.
- `docs/release/GEN1_SAVE_COVERAGE.md`: detailed Gen I range-level coverage.
- `docs/release/RELEASE_CHECKLIST.md`: public release checklist and out-of-scope list.
- `docs/CONVERSION_MODEL.md`: Red-side shared conversion model.
- `docs/RED_TO_FIRERED_MAPPING.md`: mapping principles and known future target needs.
- `docs/PROJECT_GIT_LOG.md`: public commit/milestone history.
- `docs/research/PKMN_RED_SAVE_GENIE_RESEARCH_DOCUMENT.md`: long-form research monograph.

## Final Conclusion

The Red Save Genie is complete as a preservation-grade Gen I save reader/exporter/editor foundation and as the source-side data model for future Red-to-FireRed conversion. The next major engineering frontier is FireRed: proving a byte-identical `.fred.json` round trip, implementing the FireRed writer, and validating converted saves in emulator.

