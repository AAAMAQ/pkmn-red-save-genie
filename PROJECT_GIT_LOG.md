# Project Git Log

This file records meaningful project commits and the engineering decisions behind them. Keep entries concise and factual so future work can resume from repository documentation instead of relying on long conversation history.

Note: a commit cannot contain its own final hash without an amend or follow-up commit. When an entry is created in the same commit it describes, the exact hash is reported in the completion message and is visible through `git log`.

## 2026-06-22 — Complete Red master JSON and conversion-readiness milestone

Commit hash: created by this milestone commit; see `git log --oneline` after commit.

Title: Complete Red master JSON and conversion-readiness milestone

Systems affected:

- Red master `.red.json` export/import/reconstruction.
- Legacy/test exports separation through `RedTestExports`.
- Xcode project source membership.
- Startup/export flow in `main.cpp`.
- Draft JSON Schema for Pokémon Red master saves.
- Automated `.red.json` round-trip tests.
- Conversion-readiness documentation.

Files affected:

- `Pkmn Red Save Genie/HPP Files/RedMasterJson.hpp`
- `Pkmn Red Save Genie/CPP Files/RedMasterJson.cpp`
- `Pkmn Red Save Genie/HPP Files/RedTestExports.hpp`
- `Pkmn Red Save Genie/CPP Files/RedTestExports.cpp`
- `Pkmn Red Save Genie/CPP Files/main.cpp`
- `Pkmn Red Save Genie.xcodeproj/project.pbxproj`
- `tests/red_master_json_tests.cpp`
- `docs/schema/pokemon-red-master-save.schema.json`
- `docs/RED_MASTER_JSON_CONVENTION.md`
- `docs/RED_MASTER_JSON_ROUND_TRIP.md`
- `docs/RED_SAVE_COVERAGE.md`
- `docs/RED_MASTER_JSON_COMPLETION_MILESTONE.md`
- `docs/CONVERSION_MODEL.md`
- `docs/RED_TO_FIRERED_MAPPING.md`
- `docs/POKEMON_CONVERSION_POLICY.md`
- `docs/STORY_EVENT_MAPPING.md`
- `docs/UNSUPPORTED_DATA_POLICY.md`
- `docs/FIRERED_MASTER_JSON_CONVENTION.md`
- `docs/FIRERED_SAVE_ROUND_TRIP.md`
- `docs/CONVERSION_VALIDATION.md`
- `.gitignore`

Test results:

- `red_master_json_tests`: PASS
- `savegenie_core_tests`: PASS
- Xcode build: PASS
- Draft JSON Schema validation: PASS
- `git diff --check`: PASS

Important decisions:

- `.red.json` is considered complete and stable for the Red side unless a real bug is discovered.
- `physicalImage` remains the no-edit reconstruction authority.
- `decoded` and `conversionModel` are the semantic source for future conversion work.
- Generated `.red.json` and `.fred.json` exports are ignored and should not be committed.
- FireRed `.fred.json`, FireRed reader/writer, Pokémon encryption, target mapping, converted-save generation, and emulator validation remain future phases.

Known limitations:

- FireRed save support is not implemented.
- Red-to-FireRed conversion is not implemented.
- Final legality policy for generated Gen III Pokémon fields is deferred to the conversion layer.
- Real-save conversion reliability has not been claimed.

Summary:

This milestone turned Save Genie’s Red master export into both a lossless archive and a conversion-ready semantic source. It preserves complete raw save data, reconstructs byte-identically, exposes readable Red gameplay state, and records machine-readable conversion classifications and policies for the future Red-to-FireRed pipeline.
