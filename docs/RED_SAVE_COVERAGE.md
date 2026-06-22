# Pokemon Red Save Coverage Map

Status: complete preservation coverage for `.red.json` schema `0.1.0`

The master JSON coverage system must cover every standard SRAM byte from `0x0000` through `0x7FFF`. Unknown data is allowed, but uncovered data is not.

Coverage records are byte-preservation records first. A byte can be covered as `unknown`, `runtime_state`, or `raw_preserved` before its English semantic meaning is fully decoded. The `decoded` hierarchy is the current human-readable interpretation layer, and it will continue to grow as more pret/pokered-backed metadata is verified.

Required validation:

```text
Uncovered bytes: 0
Accidental primary overlaps: 0
```

## Primary Coverage Ranges

| Start | End Inclusive | Size | Name | Classification | Decoded | Reconstruction Policy | Notes |
|---:|---:|---:|---|---|---|---|---|
| `0x0000` | `0x0497` | 1176 | Bank 0 sprite/runtime scratch buffers | `runtime_state` | no | preserve original bytes | Runtime/scratch region. |
| `0x0498` | `0x0597` | 256 | Bank 0 unused pre-Hall-of-Fame block | `unused` | no | preserve original bytes | Known unused/padding-style region. |
| `0x0598` | `0x1857` | 4800 | Hall of Fame records | `decoded` | yes | preserve original bytes | Decoded by Save Genie. |
| `0x1858` | `0x1FFF` | 1960 | Bank 0 unused post-Hall-of-Fame block | `unused` | no | preserve original bytes | Known unused/padding-style region. |
| `0x2000` | `0x2597` | 1432 | Bank 1 lead-in/runtime region | `runtime_state` | no | preserve original bytes | Partially understood runtime/overworld data. |
| `0x2598` | `0x3522` | 3979 | Bank 1 main save data checksum-covered range | `partially_decoded` | yes | preserve original bytes | Trainer, inventory, world state, events, party, current box cache, and related data. |
| `0x3523` | `0x3523` | 1 | Main checksum byte | `checksum_metadata` | yes | preserve original bytes | No-edit reconstruction preserves stored checksum exactly. |
| `0x3524` | `0x3FFF` | 2780 | Bank 1 trailing unknown/raw-preserved region | `unknown` | no | preserve original bytes | Not fully interpreted. |
| `0x4000` | `0x5A52` | 6739 | Bank 2 PC Boxes 1-6 and checksum metadata | `decoded` | yes | preserve original bytes | Permanent PC storage. |
| `0x5A53` | `0x5FFF` | 1453 | Bank 2 unused/unknown remainder | `unknown` | no | preserve original bytes | Not fully interpreted. |
| `0x6000` | `0x7A52` | 6739 | Bank 3 PC Boxes 7-12 and checksum metadata | `decoded` | yes | preserve original bytes | Permanent PC storage. |
| `0x7A53` | `0x7FFF` | 1453 | Bank 3 unused/unknown remainder | `unknown` | no | preserve original bytes | Not fully interpreted. |

## Current Interpretation

This coverage map proves preservation, not complete English translation. Several ranges remain intentionally classified as `runtime_state`, `partially_decoded`, or `unknown`. Those bytes are still stored losslessly in `.red.json`.

The current `decoded` hierarchy exports the major implemented Save Genie models: trainer/rival, options/playtime, money/coins, badges, location/runtime state, Pokédex, inventory, party, PC storage, current box cache, daycare, Hall of Fame, named events, trainer/static/story categories, scripts, missable objects, hidden items, hidden coins, visited towns, world state, and checksum statuses.

Future research may refine Bank 1 runtime fields, script semantics, missable-object categories, hidden-item categories, and unused/unknown bank tails as more `pret/pokered`-backed metadata is verified. These refinements are not required for the completed Red-side converter-source role because the bytes are already preserved and classified.
