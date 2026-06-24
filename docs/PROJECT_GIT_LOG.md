# Project Git Log

This file records meaningful project commits and the engineering decisions behind them. Keep entries concise and factual so future work can resume from repository documentation instead of relying on long conversation history.

Note: a commit cannot contain its own final hash without an amend or follow-up commit. When an entry is created in the same commit it describes, the exact hash is reported in the completion message and is visible through `git log`.

## 2026-06-24 — Keep unpublished research artifacts private

Commit hash: pending commit.

Title: Ignore unpublished research and publication drafts

Systems affected:

- Git ignore policy for unpublished manuscripts and private research material.

Files affected:

- `.gitignore`
- `docs/PROJECT_GIT_LOG.md`

Test results:

- `git check-ignore` verified the private publication and research directories.
- `git diff --check` passed.

Important decisions:

- `docs/publication/` remains local and untracked until its manuscripts are reviewed and intentionally published.
- `personal resources/` remains local and untracked because it contains private saves, references, generated outputs, and working notes.
- Publication files will be added later through an explicit reviewed commit rather than entering Git accidentally.

Known limitations:

- Ignored files are not backed up or distributed by Git.

Summary:

This repository-policy commit keeps unpublished research and personal reference material out of the public project while preserving the files locally for later verification and publication.

## 2026-06-22 — Prepare project for public research release

Commit hash: `b0728e5`

Title: Prepare project for public research release

Systems affected:

- Public documentation structure.
- Release conclusion and project status docs.
- Git/history documentation location.
- Personal Codex discussion memory handling.

Files affected:

- `README.md`
- `.gitignore`
- `docs/PROJECT_GIT_LOG.md`
- `docs/PROJECT_PUBLIC_RESEARCH_RELEASE.md`
- `docs/RED_MASTER_JSON_COMPLETION_MILESTONE.md`
- `docs/RED_MASTER_JSON_CONVENTION.md`
- `docs/RED_MASTER_JSON_ROUND_TRIP.md`
- `docs/RED_SAVE_COVERAGE.md`
- `docs/release/GEN1_SAVE_COVERAGE.md`
- `docs/release/RELEASE_CHECKLIST.md`
- `docs/research/PKMN_RED_SAVE_GENIE_RESEARCH_DOCUMENT.md`
- `docs/research/PROJECT_TIMELINE.md`
- `docs/research/REFERENCES.md`
- `docs/research/OPEN_RESEARCH_QUESTIONS.md`
- `docs/external_projects_deep_analysis.md`

Test results:

- Documentation-only pass; no production code changed.
- Build/test rerun deferred unless requested after review.

Important decisions:

- The Red Save Genie is documented as complete for its intended Red-side research, verification, archival, and converter-source role.
- `.red.json` remains stable unless a real bug is discovered.
- Human-reported verification/playtesting/emulator results are accepted for this documentation pass unless contradicted by repository evidence.
- FireRed `.fred.json`, FireRed reader/writer, Gen III serialization, conversion output, and emulator validation remain future phases.
- `docs/CODEX_CHAT_VERIFICATION_AND_DISCUSSIONS.md` is personal and ignored; it must not be committed as a required public project document.

Known limitations:

- This pass does not add new parser behavior, tests, or FireRed support.
- Broader public-release validation can still add evidence, but should not reopen the completed Red master JSON design without a concrete bug.

Summary:

This milestone prepares the repository for public research review by consolidating the final Red-side conclusion, correcting stale roadmap/status language, cross-linking authoritative docs, preserving project history under `docs/`, and separating private conversation memory from public documentation.

## 2026-06-22 — Complete Red master JSON and conversion-readiness milestone

Commit hash: `929b6a0`

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
