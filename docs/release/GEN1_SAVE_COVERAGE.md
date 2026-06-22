# Gen I Save Coverage Map

Date: 2026-06-20

Status update: this detailed coverage map remains the authoritative range-level view for the Gen I Save Genie release. The Red-side project is complete for preservation and converter-source use because every standard SRAM byte is preserved/classified and the major transferable gameplay concepts are decoded. This document still intentionally distinguishes decoded gameplay data from runtime, scratch, padding, unused, and unknown bytes.

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
| `0x2611-0x278D` | 1 | Partially decoded/exported | `pret/pokered` map/area runtime fields are now labeled for last map, current map header, map dimensions, pointers, connection flags, map background tile, warp count, destination warp ID, sign count, and sprite count. Connection structs, warp-entry payloads, sign coordinates/text IDs, and object payloads remain runtime/cache data and are not fully interpreted per entry. |
| `0x278E-0x278F` | 1 | Decoded | Offset since last special warp X/Y. |
| `0x2790-0x27D3` | 1 | Partially decoded/exported | Map sprite movement/extra-data cache is identified from `pret/pokered`; current map height2/width2 and VRAM pointer are exported. Individual cached sprite rows are not yet translated into stable object records. |
| `0x27D4-0x27D6` | 1 | Decoded | Player movement/current/last-stop direction bytes. |
| `0x27D7-0x27E5` | 1 | Known runtime / not exported | Tileset bank/pointers, talking-over tiles, grass tile, and short padding area from `pret/pokered`; not meaningful as standalone release data yet. |
| `0x27E6-0x284D` | 1 | Decoded | PC item box item/quantity pairs. |
| `0x284E` | 1 | Decoded | Hall of Fame record-count hint. |
| `0x284F` | 1 | Partially understood | Byte adjacent to HoF count/coins; retained as miscellaneous until verified. |
| `0x2850-0x2851` | 1 | Decoded | Slot-machine coins, 2-byte BCD. |
| `0x2852-0x286E` | 1 | Decoded | Missable object/NPC/object-state used flags with 228 named rows, locations, sprite indexes, default show/hide state, and true/false toggled-off state. |
| `0x286F-0x2871` | 1 | Known but not exported | Spare/unused missable flag bytes according to Junebug v2 used-count model. |
| `0x2872-0x289B` | 1 | Known runtime / partially exported | Saved sprite image index, toggleable-object list, and padding before current-script region are identified from `pret/pokered`; the toggleable-object list is still not expanded into per-current-map rows. |
| `0x289C-0x2915` | 1 | Decoded | 97 named current script-progress values using `pret/pokered` symbol order cross-checked against Junebug/Twilight metadata. |
| `0x2916-0x299B` | 1 | Known padding/reserved script region | Remaining bytes after the 97 named script values; preserved and included in raw nonzero-byte count but not assigned fake meanings. |
| `0x299C-0x29A2` | 1 | Decoded | 54 hidden-item collected flags with map names and X/Y coordinates from `pret/pokered` `hidden_item_coords.asm`. |
| `0x29A3-0x29A9` | 1 | Known but not exported | Spare/unused hidden-item bytes according to Junebug v2 used-count model. |
| `0x29AA-0x29AB` | 1 | Decoded | 12 Game Corner hidden-coin flags with X/Y coordinates from `pret/pokered` `hidden_coins.asm`. |
| `0x29AC` | 1 | Decoded | Walk/bike/surf movement-mode byte. |
| `0x29AD-0x29B6` | 1 | Known padding/runtime / not exported | Short runtime/padding area between movement mode and Fly destination flags. |
| `0x29B7-0x29B8` | 1 | Decoded | 11 visited town/Fly-destination flags with city names. |
| `0x29B9-0x29BA` | 1 | Decoded | Safari steps, big-endian word. |
| `0x29BB-0x29BC` | 1 | Decoded | Fossil item handed to Cinnabar Lab and revived fossil species/result byte. |
| `0x29BD-0x29BF` | 1 | Known padding/runtime / not exported | Short padding/runtime bytes before jumping/special map state. |
| `0x29C0` | 1 | Decoded | Player jumping Y-screen coordinate byte. |
| `0x29C1-0x29D3` | 1 | Partially decoded/exported | Last blackout map, special warp destination map, dungeon warp destination map, and dungeon-warp index are exported. Adjacent collision/boulder/unused/padding bytes remain identified but not deeply interpreted. |
| `0x29D4` | 1 | Decoded | Player/story bits: Strength, Surf, rods, Saffron guards, Card Key. |
| `0x29D5` | 1 | Known padding/runtime / not exported | Adjacent status padding byte from `pret/pokered`; no standalone user-facing meaning assigned. |
| `0x29D6` | 1 | Decoded | Badge mirror bitfield, kept in sync by Safe Editor badge writes. |
| `0x29D7-0x29D8` | 1 | Known padding/runtime / not exported | Adjacent status padding bytes from `pret/pokered`; no standalone user-facing meaning assigned. |
| `0x29D9-0x29DA` | 1 | Decoded | Battle/runtime flags, scripted/dungeon warp bits, NPC-facing bit, Lapras/heal/starter/link/no-battle bits, and scripted-NPC movement bit. |
| `0x29DB` | 1 | Known padding/runtime / not exported | Adjacent status padding byte from `pret/pokered`. |
| `0x29DC` | 1 | Decoded | Text/runtime flag byte: NPC sprite movement, ignore joypad, no-letter-delay, and joypad simulation bits. |
| `0x29DD` | 1 | Known padding/runtime / not exported | Adjacent status padding byte from `pret/pokered`. |
| `0x29DE` | 1 | Decoded | Playtime/debug/warp-control byte: count playtime, debug mode, Fly/dungeon warp state, force bike, and blackout destination bits. |
| `0x29DF` | 1 | Decoded | Test battle, skip joypad check warps, trainer-wants-battle, current-map-next-frame, and Fly-out-of-battle bits. |
| `0x29E0` | 1 | Decoded | Elite/story bit byte; Lorelei defeated bit exported. |
| `0x29E1` | 1 | Known padding/runtime / not exported | Adjacent status padding byte from `pret/pokered`. |
| `0x29E2` | 1 | Decoded | Door/warp/ledge/spin player bits exported. |
| `0x29E3-0x29F2` | 1 | Partially decoded/exported | Completed in-game trade flags, warped-from warp/map, Card Key door coordinates, and Lt. Surge trash-can lock indexes are exported. Padding bytes inside the range remain preserved/not assigned fake meanings. |
| `0x29F3-0x2B32` | 1 | Decoded | Event flag bitset; raw counts, complete known `pret/pokered` true/false label list, trainer-progress rows, story categories, static battle flags, and gym/badge consistency checks. |
| `0x2B33-0x2CEC` | 1 | Partially decoded/exported | Wild/link/enemy-party union and trainer/runtime area. Save Genie exports trainer header pointer, Cinnabar quiz wrong-answer opponent, and live current-map script index; enemy party/link/wild union bytes remain out of release scope. |
| `0x2CED-0x2CF1` | 1 | Decoded | Playtime hours/minutes/seconds. |
| `0x2CF2-0x2CF3` | 1 | Decoded | Safari game-over byte and Safari Ball count. |
| `0x2CF4-0x2D2C` | 1 | Decoded | Daycare in-use flag, nickname, OT name, and deposited boxed-Pokemon-style record. |
| `0x2D2D-0x2F2B` | 1 | Partially understood | Main save data not fully exported yet. |
| `0x2F2C-0x30BF` | 1 | Decoded | Party count, species list, party structs, OT names, nicknames. |
| `0x30C0-0x3521` | 1 | Decoded | Current Box Cache using full box layout. |
| `0x3522` | 1 | Checksum-covered data | Last byte included in main checksum range. |
| `0x3523` | 1 | Checksum metadata | Main checksum byte. |
| `0x3524-0x3FFF` | 1 | Unknown / not exported | No stable user-facing persistent Gen I save meaning was confirmed from current `pret/pokered`/Junebug pass; preserved untouched and excluded from English translation. |
| `0x4000-0x5A4B` | 2 | Decoded | Permanent PC Boxes 1-6. |
| `0x5A4C` | 2 | Checksum metadata | Bank 2 all-box checksum. |
| `0x5A4D-0x5A52` | 2 | Checksum metadata | Per-box checksums for Boxes 1-6. |
| `0x5A53-0x5FFF` | 2 | Known unused / not exported | Bank 2 remainder. |
| `0x6000-0x7A4B` | 3 | Decoded | Permanent PC Boxes 7-12. |
| `0x7A4C` | 3 | Checksum metadata | Bank 3 all-box checksum. |
| `0x7A4D-0x7A52` | 3 | Checksum metadata | Per-box checksums for Boxes 7-12. |
| `0x7A53-0x7FFF` | 3 | Known unused / not exported | Bank 3 remainder. |

## Remaining Coverage Work

- Optional refinement: replace some broad `partially understood` runtime ranges with smaller named ranges if future `pret/pokered` evidence proves stable user-facing meanings.
- Optional refinement: decide whether current-map warp/sign/sprite cache rows should become structured diagnostics or remain raw-preserved runtime data.
- Future research: determine whether bytes in `0x3524-0x3FFF` have stable persistent meaning in Red/Blue saves; do not assign names without a source.
- Future converter work: translate broad story categories into FireRed target policies before writing FireRed state.
- Maintenance: add automated coverage assertions if range definitions begin changing frequently.
- Boundary: FireRed/Gen III ranges stay out of this Gen I coverage report.

These items do not block the completed Red-side Save Genie milestone. They are refinements, future converter dependencies, or evidence-expansion tasks rather than missing source preservation.

## Reference Credits

The world/player/daycare coverage split was cross-checked against the Apache-2.0 Junebug/Twilight editor repositories:

- `junebug12851/pokered-save-editor`
- `junebug12851/pokered-save-editor-2`

Those projects are used as research references and inspiration. `pret/pokered` remains the authority for event-symbol names and game behavior; the current event/trainer flag report is derived from `pret/pokered` `constants/event_constants.asm` and trainer script usage.
