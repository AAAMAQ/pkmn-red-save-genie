# Gen I Save Coverage Map

Date: 2026-06-20

This report tracks the current Save Genie understanding of the 32 KiB Gen I save range `0x0000-0x7FFF`. It is intentionally conservative: a range is only marked decoded when the current C++ reader exports or summarizes it.

## Coverage Status

| Range | Bank | Status | Current handling |
| --- | --- | --- | --- |
| `0x0000-0x0497` | 0 | Known scratch/runtime | Sprite/runtime buffers; not exported. |
| `0x0498-0x0597` | 0 | Known unused | Not exported. |
| `0x0598-0x1857` | 0 | Decoded | Hall of Fame records, species, level, nickname. |
| `0x1858-0x1FFF` | 0 | Known unused | Not exported. |
| `0x2000-0x2597` | 1 | Partially understood | Main-data lead-in/runtime regions; not fully mapped. |
| `0x2598-0x25C8` | 1 | Decoded | Trainer name, Pokédex owned/seen, checksummed range start. |
| `0x25C9-0x25F2` | 1 | Decoded | Bag item count and item/quantity pairs. |
| `0x25F3-0x260E` | 1 | Decoded | Money, rival name, options byte, badges, letter delay, trainer ID, music bytes, contrast, map, X/Y. |
| `0x260F-0x2610` | 1 | Decoded | Player block coordinates X/Y. |
| `0x2611-0x278D` | 1 | Partially understood | Bank 1 overworld/runtime range; not fully exported yet. |
| `0x278E-0x278F` | 1 | Decoded | Offset since last special warp X/Y. |
| `0x2790-0x27D3` | 1 | Partially understood | Area/runtime data not fully mapped yet. |
| `0x27D4-0x27D6` | 1 | Decoded | Player movement/current/last-stop direction bytes. |
| `0x27D7-0x27E5` | 1 | Partially understood | Area/runtime tail before PC item box. |
| `0x27E6-0x284D` | 1 | Decoded | PC item box item/quantity pairs. |
| `0x284E` | 1 | Decoded | Hall of Fame record-count hint. |
| `0x284F-0x2851` | 1 | Partially understood | Miscellaneous main save fields around coins/missables; not fully exported yet. |
| `0x2852-0x286E` | 1 | Decoded | Missable object/NPC/object-state used flags, counted read-only. |
| `0x286F-0x2871` | 1 | Known but not exported | Spare/unused missable flag bytes according to Junebug v2 used-count model. |
| `0x2872-0x289B` | 1 | Partially understood | Miscellaneous world/area bytes before current-script region. |
| `0x289C-0x299B` | 1 | Partially decoded | Current script-progress byte region; Save Genie counts nonzero bytes but does not yet import all 97 script metadata names. |
| `0x299C-0x29A2` | 1 | Decoded | Hidden item collected used flags, counted read-only. |
| `0x29A3-0x29A9` | 1 | Known but not exported | Spare/unused hidden-item bytes according to Junebug v2 used-count model. |
| `0x29AA-0x29AB` | 1 | Decoded | Hidden coin collected flags, counted read-only. |
| `0x29AC` | 1 | Decoded | Walk/bike/surf movement-mode byte. |
| `0x29AD-0x29B6` | 1 | Partially understood | Miscellaneous world/player state bytes not fully exported yet. |
| `0x29B7-0x29B8` | 1 | Decoded | Visited town/Fly-destination used flags. |
| `0x29B9-0x29BA` | 1 | Decoded | Safari steps, big-endian word. |
| `0x29BB-0x29BF` | 1 | Partially understood | Miscellaneous world/player state bytes. |
| `0x29C0` | 1 | Decoded | Player jumping Y-screen coordinate byte. |
| `0x29C1-0x29D3` | 1 | Partially understood | Miscellaneous world/player state bytes. |
| `0x29D4` | 1 | Decoded | Player/story bits: Strength, Surf, rods, Saffron guards, Card Key. |
| `0x29D5` | 1 | Partially understood | Adjacent story/player byte not fully exported. |
| `0x29D6` | 1 | Decoded | Badge mirror bitfield, kept in sync by Safe Editor badge writes. |
| `0x29D7-0x29D8` | 1 | Partially understood | Miscellaneous story/player bytes not fully exported. |
| `0x29D9-0x29DA` | 1 | Decoded | Battle/runtime flags plus Lapras/heal/starter/link/no-battle bits. |
| `0x29DB` | 1 | Partially understood | Miscellaneous story/player byte. |
| `0x29DC` | 1 | Decoded | Text flag byte; no-letter-delay bit exported. |
| `0x29DD` | 1 | Partially understood | Miscellaneous story/player byte. |
| `0x29DE` | 1 | Decoded | Playtime control byte; count-playtime bit exported. |
| `0x29DF` | 1 | Decoded | Fly-out-of-battle bit exported. |
| `0x29E0` | 1 | Decoded | Elite/story bit byte; Lorelei defeated bit exported. |
| `0x29E1` | 1 | Partially understood | Miscellaneous story/player byte. |
| `0x29E2` | 1 | Decoded | Door/warp/ledge/spin player bits exported. |
| `0x29E3-0x29F2` | 1 | Partially understood | Miscellaneous main save fields; not fully exported yet. |
| `0x29F3-0x2B32` | 1 | Decoded | Event flag bitset; raw counts, complete known `pret/pokered` true/false label list, trainer-progress rows, story categories, static battle flags, and gym/badge consistency checks. |
| `0x2B33-0x2CEC` | 1 | Partially understood | Story/script/object state not fully mapped yet. |
| `0x2CED-0x2CF1` | 1 | Decoded | Playtime hours/minutes/seconds. |
| `0x2CF2-0x2CF3` | 1 | Decoded | Safari game-over byte and Safari Ball count. |
| `0x2CF4-0x2D2C` | 1 | Decoded | Daycare in-use flag, nickname, OT name, and deposited boxed-Pokemon-style record. |
| `0x2D2D-0x2F2B` | 1 | Partially understood | Main save data not fully exported yet. |
| `0x2F2C-0x30BF` | 1 | Decoded | Party count, species list, party structs, OT names, nicknames. |
| `0x30C0-0x3521` | 1 | Decoded | Current Box Cache using full box layout. |
| `0x3522` | 1 | Checksum-covered data | Last byte included in main checksum range. |
| `0x3523` | 1 | Checksum metadata | Main checksum byte. |
| `0x3524-0x3FFF` | 1 | Unknown / not exported | Outside current primary decoded ranges. |
| `0x4000-0x5A4B` | 2 | Decoded | Permanent PC Boxes 1-6. |
| `0x5A4C` | 2 | Checksum metadata | Bank 2 all-box checksum. |
| `0x5A4D-0x5A52` | 2 | Checksum metadata | Per-box checksums for Boxes 1-6. |
| `0x5A53-0x5FFF` | 2 | Known unused / not exported | Bank 2 remainder. |
| `0x6000-0x7A4B` | 3 | Decoded | Permanent PC Boxes 7-12. |
| `0x7A4C` | 3 | Checksum metadata | Bank 3 all-box checksum. |
| `0x7A4D-0x7A52` | 3 | Checksum metadata | Per-box checksums for Boxes 7-12. |
| `0x7A53-0x7FFF` | 3 | Known unused / not exported | Bank 3 remainder. |

## Remaining Coverage Work

- Replace broad `partially understood` regions with smaller named ranges from `pret/pokered`.
- Import the full 97-entry script metadata table if script names become required in JSON.
- Import more specific source names for completed-script bytes, missable objects, hidden items, hidden coins, and NPC object states where `pret/pokered` provides authoritative metadata.
- Convert broad story categories into a reviewed conversion-policy table before using them for Red-to-FireRed state writing.
- Add automated coverage assertions so decoded ranges stay synchronized with source constants.
- Keep FireRed/Gen III ranges out of this Gen I coverage report.

## Reference Credits

The world/player/daycare coverage split was cross-checked against the Apache-2.0 Junebug/Twilight editor repositories:

- `junebug12851/pokered-save-editor`
- `junebug12851/pokered-save-editor-2`

Those projects are used as research references and inspiration. `pret/pokered` remains the authority for event-symbol names and game behavior; the current event/trainer flag report is derived from `pret/pokered` `constants/event_constants.asm` and trainer script usage.
