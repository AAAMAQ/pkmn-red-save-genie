# Red Master JSON Completion Milestone

Date: 2026-06-22
Status: complete and stable unless a real bug is discovered
Schema version: `0.1.0` draft, stable enough for the completed Red-side milestone

## Summary

The Pokémon Red `.red.json` master-save system is now considered complete for the Red side of Save Genie.

It provides two coordinated views of the same source save:

- `physicalImage`: the archival authority used for exact no-edit reconstruction.
- `decoded` and `conversionModel`: the readable and conversion-ready semantic source for future Red-to-FireRed work.

The Red master file preserves the original save bytes, the standard `0x0000-0x7FFF` SRAM region, trailing bytes beyond `0x8000`, hashes, coverage metadata, checksum information, decoded gameplay data, and conversion-readiness guidance.

## What Was Implemented

- Lossless `.sav -> .red.json -> [RECONSTRUCTED] .sav` round trip.
- Preservation of full `0x8000` SRAM plus trailing bytes, including known `0x802C` style files.
- Canonical uppercase continuous hex physical image.
- SHA-256 hashes for whole file, standard SRAM, and trailing data.
- Coverage map with `uncoveredBytes: 0` and `overlappingPrimaryBytes: 0`.
- Expanded `decoded` hierarchy for trainer, rival, options, playtime, money, coins, badges, location, runtime state, Pokédex, inventory, party, PC storage, current box cache, daycare, Hall of Fame, events, trainer battles, static battles, story progress, scripts, missables, hidden items, hidden coins, visited towns, world state, and checksums.
- Top-level `conversionModel` describing the future pipeline:

```text
.red.json
  -> shared conversion model
  -> .fred.json
  -> FireRed save writer
```

- Per-Pokémon conversion metadata for party, permanent boxes, current box cache, and daycare records.
- Draft deterministic policies for DV-to-IV and Stat Experience-to-EV conversion.
- Generated Gen III field policy placeholders for PID, nature, gender, ability, friendship, met location, Poké Ball, language, and ribbons.
- Inventory conversion classification for direct items, TMs/HMs, key/progression items, and unsupported cases.
- Pokédex FireRed mapping metadata using National Dex identity rather than Gen I internal IDs.
- Draft JSON Schema and automated synthetic tests for round-trip, validation failure, deterministic export, schema validation, and semantic expansion.

## Why This Is Considered Complete

The Red side is complete because it now satisfies the core Save Genie requirements:

- It preserves every original byte needed to reconstruct the source save.
- It keeps generated exports separate from the canonical master format.
- It exposes the meaningful Red gameplay state in a readable structure.
- It separates raw archival truth from semantic conversion truth.
- It classifies transferable gameplay concepts for future conversion.
- It documents what is direct, semantic, Red-only, unsupported, approximate, or policy-required.
- It passes synthetic round-trip tests with zero byte differences for standard and trailing-byte saves.

The remaining draft fields are intentional and do not mean the Red decoder is incomplete.

For public research-release documentation, MAQ's reported human verification, playtesting, emulator checks, and bug-testing results are accepted as release evidence unless current repository evidence contradicts them. Additional validation may improve confidence, but should not reopen the completed Red master JSON design without a concrete bug.

## Intentionally Deferred

These are future conversion-policy or FireRed-side tasks, not blockers for the completed Red master JSON:

- Pokémon type lookup, because types can be inferred later from species identity.
- Maximum move PP, because it can be inferred later from move identity and PP Ups.
- Final DV-to-IV and Stat Experience-to-EV policy review.
- PID, nature, gender, ability, friendship, met location, Poké Ball, language, and ribbons, because they are Gen III target fields.
- FireRed target item IDs, move IDs, species IDs, map IDs, flags, variables, and save offsets.
- FireRed save slots, sections, checksums, security key, Pokémon encryption, and substructure permutation.
- `.fred.json` implementation.
- Red-to-FireRed converted save generation.
- Emulator load/save-again validation.

## Validation Evidence

Latest synthetic tests:

| Fixture | Source Size | Reconstructed Size | SHA-256 Result | Byte Differences |
|---|---:|---:|---|---:|
| `standard.sav` | `0x8000` | `0x8000` | identical | 0 |
| `trailing.sav` | `0x802C` | `0x802C` | identical | 0 |
| `semantic.sav` | `0x8000` | `0x8000` | identical | 0 |

Test commands run during milestone completion:

- `red_master_json_tests`: PASS
- `savegenie_core_tests`: PASS
- Xcode build for `Pkmn Red Save Genie`: PASS
- Draft JSON Schema validation for generated standard/trailing/semantic `.red.json`: PASS
- `git diff --check`: PASS

## Future Phase Boundary

The next phase is not to redesign `.red.json`. The next phase is to build the FireRed side and the cross-generation converter:

1. Define `.fred.json`.
2. Implement FireRed reader and byte-identical FireRed round trip.
3. Implement FireRed writer, section checksums, save-slot handling, and Pokémon encryption.
4. Finalize mapping tables and conversion policies.
5. Generate converted FireRed saves.
6. Validate converted saves in emulator and by save-again testing.

## Public Release Documentation Links

- `docs/PROJECT_PUBLIC_RESEARCH_RELEASE.md`: final public research-release conclusion and documentation map.
- `docs/RED_MASTER_JSON_CONVENTION.md`: canonical `.red.json` schema and data-authority rules.
- `docs/RED_MASTER_JSON_ROUND_TRIP.md`: reconstruction workflow and test harness.
- `docs/RED_SAVE_COVERAGE.md`: master JSON coverage rules.
- `docs/release/GEN1_SAVE_COVERAGE.md`: detailed Gen I byte-range coverage.
- `docs/CONVERSION_MODEL.md`: Red-side conversion-source model.
