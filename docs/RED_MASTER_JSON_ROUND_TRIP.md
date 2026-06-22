# Red Master JSON Round Trip

Status: draft
Schema version: `0.1.0`

## Goal

The P0 milestone proves:

```text
.sav -> .red.json -> [RECONSTRUCTED] .sav
```

with:

- Identical file size.
- Identical SHA-256.
- Zero byte differences.
- Preserved trailing bytes beyond `0x8000`.

## Export Sequence

1. Load the original save bytes.
2. Require at least `0x8000` bytes.
3. Split the file into:
   - `standardSram`: first `0x8000` bytes.
   - `trailingData`: every byte after `0x7FFF`.
4. Encode both byte ranges as uppercase continuous hex.
5. Compute SHA-256 using CommonCrypto.
6. Write collision-safe `<save-name>.red.json`.

## Reconstruction Sequence

1. Load `.red.json`.
2. Parse JSON syntax.
3. Validate `schema.format == "pkmn-red-master-save"`.
4. Validate `schema.schemaVersion == "0.1.0"`.
5. Validate physical-image encoding.
6. Decode canonical hex.
7. Require `standardSramHex` to decode to exactly `0x8000` bytes.
8. Validate declared total/trailing lengths.
9. Recompute and compare hashes.
10. Write collision-safe `[RECONSTRUCTED] <save-name>.sav`.
11. Compare against the original save when available.

## Semantic Expansion Guardrail

The expanded `decoded` section is intentionally ignored by no-edit reconstruction. Round-trip verification proves that adding trainer, party, box, Pokédex, event, world-state, Hall of Fame, daycare, and runtime semantics does not alter `physicalImage`.

If semantic consistency checks fail, the exporter reports diagnostics, but the reconstructor still uses only the validated raw physical image. A semantic inconsistency is not allowed to rewrite, repair, or normalize bytes during a no-edit rebuild.

## Stop Rule

If no-edit byte-identical reconstruction fails, do not continue into semantic/schema polish. Report the exact blocker first.

## Current Test Harness

`tests/red_master_json_tests.cpp` uses synthetic saves only:

- Standard `0x8000` save round trip.
- `0x802C` trailing-byte save round trip.
- Missing required fields.
- Unsupported schema version.
- Hash mismatch.
- Invalid canonical hex.
- Collision-safe reconstructed output.
- Deterministic export mode with volatile timestamp disabled.
- Expanded decoded hierarchy presence.
- Semantic fixture with trainer, rival, money, coins, badges, Pokédex, inventory, party Pidgey/PEGGY, current box cache, events, world-state sections, and semantic consistency fields.
- Reconfirmation that semantic expansion still reconstructs byte-identically.

No private save files or ROMs are required.

## Latest Synthetic Results

The current synthetic test harness verifies:

| Fixture | Source Size | Reconstructed Size | Expected Result |
|---|---:|---:|---|
| `standard.sav` | `0x8000` | `0x8000` | zero byte differences |
| `trailing.sav` | `0x802C` | `0x802C` | zero byte differences, trailing bytes preserved |
| `semantic.sav` | `0x8000` | `0x8000` | zero byte differences after expanded decoded export |

Use the command-line test output as the authoritative current result for exact hashes because temporary synthetic fixtures are regenerated on each run.
