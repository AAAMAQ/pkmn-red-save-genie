# Pokemon Red Master JSON Convention

Status: Red-side complete; draft schema
Schema version: `0.1.0`
Canonical extension: `.red.json`

## Purpose

`<save-name>.red.json` is the canonical, lossless Pokémon Red master-save representation for Save Genie. It is different from `PokemonBoxes.json`, `PokemonSummary.json`, and `SaveGenieSummary.txt`, which are legacy/test exports for inspection and regression work.

The first requirement is archival reversibility:

```text
original.sav -> original.red.json -> [RECONSTRUCTED] original.sav
```

An unchanged `.red.json` must reconstruct a byte-identical `.sav`.

## Draft Schema Identity

The development schema uses:

```json
{
  "schema": {
    "format": "pkmn-red-master-save",
    "schemaVersion": "0.1.0",
    "canonicalExtension": ".red.json",
    "lossless": true,
    "stability": "draft"
  }
}
```

Do not promote this to `1.0.0` until a future public schema-freeze review. The `0.1.0` schema is complete for the current Red-side Save Genie milestone, but it remains versioned as a draft so future FireRed/converter requirements can add compatible fields carefully.

## Canonical Byte Encoding

Raw save bytes are stored in `physicalImage` using canonical hexadecimal:

- Uppercase only.
- Continuous string.
- Two hex characters per byte.
- No embedded whitespace.
- Importer rejects lowercase hex, invalid characters, odd length, wrong decoded length, and hash mismatches.

The physical image is the reconstruction authority in schema `0.1.0`.

## Reconstruction Precedence

No-edit reconstruction uses only:

```text
physicalImage.standardSramHex + physicalImage.trailingDataHex
```

Semantic fields are informational in `0.1.0`. The reconstructor does not repair checksums, normalize unused bytes, or rebuild the save from decoded objects.

Future edited reconstruction should apply explicit supported semantic edits onto a copy of the original raw bytes, then update only required dependent values such as checksums.

## Raw Versus Semantic Authority

`physicalImage` is the archival truth for no-edit reconstruction. It preserves the exact original bytes, including unused bytes, runtime bytes, unknown regions, checksums, and trailing data.

`decoded` is the semantic truth for inspection, comparison, future editing, and future Red-to-FireRed conversion. It normalizes the useful data previously spread across `PokemonSummary.json`, `PokemonBoxes.json`, `SaveGenieSummary.txt`, and terminal output into one canonical hierarchy.

In schema `0.1.0`, semantic validation failures are diagnostic. They must not mutate or block raw reconstruction unless the physical image itself is malformed.

## Trailing Bytes

The standard Gen I SRAM region is `0x0000-0x7FFF` (`0x8000` bytes). Files larger than `0x8000`, such as `0x802C`, are decoded using the first `0x8000` bytes and preserve trailing bytes exactly.

Trailing data is not interpreted unless future research proves its meaning. It is currently classified as `unknown-preserved`.

## Top-Level Sections

- `schema`: format identity, draft version, game/generation assumptions.
- `source`: original filename, size, hashes, warnings, generation metadata.
- `integrity`: known Gen I checksum data.
- `physicalImage`: canonical byte payload used for reconstruction.
- `coverage`: primary coverage ranges for `0x0000-0x7FFF`.
- `decoded`: current Save Genie semantic data, partial and informational.
- `conversionModel`: layout-independent Red-to-FireRed source model, classification table, mapping policies, and conversion warnings.
- `unknownData`: unknown/raw-preserved range references.
- `reconstruction`: no-edit reconstruction policy.
- `diagnostics`: export and coverage status.

## Decoded Hierarchy

The current draft emits these decoded sections when `includeDecodedSummary` is enabled:

- `trainer`: player name, trainer ID, raw name bytes, and source offsets.
- `rival`: rival name, raw bytes, and source offsets.
- `options`: options, text speed, contrast, and related player-state bytes.
- `playtime`: hours, minutes, seconds, frames, and playtime flags.
- `moneyAndCoins`: packed-BCD money and confirmed coin value.
- `badges`: readable badge names, owned booleans, bit indices, and raw bytes.
- `location`: current map ID/name, coordinates, last map, movement mode, and map dimensions where known.
- `runtimeState`: safari state, movement/runtime flags, warp/sprite counts, and other runtime values.
- `pokedex`: owned/seen totals, owned/seen lists, and one entry for each National Dex species.
- `inventory`: bag and PC item storage with item names, IDs, quantities, slots, and offsets.
- `party`: party count and detailed Pokémon records.
- `pcStorage`: all twelve permanent PC boxes with detailed Pokémon records.
- `currentBoxCache`: selected box byte, cache/permanent comparison, synchronization status, and cache contents without exposing internal box `-1` as a public box number.
- `daycare`: daycare occupancy and Pokémon data when present.
- `hallOfFame`: Hall of Fame entries and Pokémon records.
- `events`: named event flags with categories, descriptions, persistence class, and source metadata.
- `trainerBattles`: trainer flag summary and detailed trainer/story/static flag category data.
- `staticBattles`: static/legendary encounter summary.
- `storyProgress`: major story and gym-consistency summaries.
- `scripts`: current script values and verified/unknown interpretation status.
- `missableObjects`: named missable object states.
- `hiddenItems`: named hidden item collection states.
- `hiddenCoins`: hidden Game Corner coin states.
- `visitedTowns`: visited town/Fly destination states.
- `worldState`: runtime/world-state summaries and raw field metadata.
- `checksums`: decoded checksum validity values.

The decoded hierarchy is a normalized superset of the legacy exports, not an embedded copy of those files.

## Metadata, Confidence, And Persistence

Where currently available, decoded records include offsets, raw hex, lengths, bit indices, encodings, source symbols, source references, confidence, persistence class, and notes.

Confidence values currently include:

- `verified`: implemented and checked against current parser behavior or direct save structure knowledge.
- `strongly_supported`: backed by pret/pokered symbols or project research, but still needs broader real-save validation.
- `inferred`: derived from other evidence, such as Hall of Fame implying Elite Four completion.
- `unknown`: preserved but not semantically interpreted.
- `conflicting`: reserved for future validation when raw evidence disagrees.

Persistence values currently include:

- `persistent`: intended long-lived save progress.
- `temporary`: transient room or battle state that may reset.
- `runtime`: runtime/overworld state bytes.
- `map_local`: map-local object/script state.
- `unknown`: preserved but not classified yet.

## Duplicate Data Policy

Some information appears in more than one semantic view. For example, named event flags appear in `events` while category sections appear in `trainerBattles`, `staticBattles`, and `storyProgress`. The canonical event records remain the named flag list; category sections are filtered summaries and cross-reference views.

Legacy exports remain available through `RedTestExports` for regression/debugging. They are not the canonical master save format.

## Conversion Model

`conversionModel` is the bridge from Red-specific decoded data toward the future converter:

```text
.red.json decoded data
  -> conversionModel
  -> shared conversion model
  -> .fred.json
  -> FireRed save writer
```

This section intentionally avoids FireRed save offsets. It classifies concepts as `directTransfer`, `semanticTranslation`, `redOnlyPreservation`, or `unsupportedOrPolicyRequired`, and records mapping policies for species, moves, items, text, status, DVs/IVs, Stat Experience/EVs, and generated Gen III Pokémon fields.

Current status: Red-side conversion-readiness is complete for source modeling. FireRed `.fred.json`, FireRed section writing, Pokémon encryption, and converted-save generation are not implemented yet.

## Versioning

- `0.x`: draft development; fields may change.
- `1.0.0`: first reviewed stable schema.
- Patch releases should not remove fields.
- Minor releases may add backward-compatible fields.
- Major releases may change reconstruction or semantic precedence rules.

## External Credit

Save Genie uses current local source code as the implementation authority. Research and cross-checking are informed by pret/pokered and Junebug's Pokémon Red save editor repositories. External code should not be copied unless licensing permits it and attribution is recorded.

## Public Release Boundary

The `.red.json` design should be treated as stable for Red-side preservation and conversion-source use unless a real bug is found. Future work should extend the pipeline through `.fred.json` and FireRed reader/writer modules rather than redesigning the Red master format without evidence.
