# External Projects Deep Analysis

Analysis date: 2026-06-19

Repository under development: `pkmn-red-save-genie`

Local project rule: the checked-out local repository is the source of truth for current behavior. External projects are reference material unless explicitly integrated later.

Claim labels used in this document:

| Label | Meaning |
| --- | --- |
| Confirmed from code | A file was opened or searched and the claim is based on inspected source. |
| Confirmed from documentation | A README, docs page, package metadata, or license file states the claim. |
| Likely inference | The claim follows from inspected evidence, but was not directly stated. |
| Unknown / not verified | The research pass did not establish the claim. |

## 1. Executive Summary

No analyzed project was found to implement full whole-save Pokemon Red/Blue Gen I to Pokemon FireRed Gen III conversion. The ecosystem has strong pieces, but not the complete pipeline this project is aiming toward. The pieces are complementary:

| Area | Best inspected source | Finding |
| --- | --- | --- |
| Broad Gen I save editing | `junebug12851/pokered-save-editor` | Broad Angular/Electron save editor with current-box handling, main and box checksums, world/event sections, party, boxes, Hall of Fame, text tables, and editable UI concepts. |
| Browser-local save workflow | `RyudoSynbios/game-tools-collection` | SvelteKit browser tool with declarative schemas, local file load, Blob download, checksum update hooks, and Red/Blue/Yellow variant handling. |
| Pokemon-level Gen I/II to Gen III conversion | `Pokemon-Community-Conversion-Standard` | C++ conversion library concept; inspected code currently implements the "Original" method path, with important policy logic for PID, nature, IVs, moves, shininess, friendship, met data, and mythical sanitization. |
| Hardware Pokemon transfer | `Poke_Transporter_GB` | GBA multiboot program that uses PCCS and writes Gen III save sectors/scripts on hardware; it is a transporter, not a whole-save converter. |
| Authoritative Gen I symbols | `pret/pokered` | Best source for SRAM layout, save routines, event constants, trainer defeated event names, maps, items, species, moves, and text encoding. |
| Authoritative FireRed symbols | `pret/pokefirered` | Best source for save sections, two-slot save selection, checksums, SaveBlock layout, Pokemon structures, flags, variables, maps, Sevii state, and National Dex state. |
| Current local project | local `pkmn-red-save-genie` | Strong Gen I reader/exporter foundation: trainer summary, checksums, Hall of Fame, Pokedex, inventory, raw event summary, party, PC boxes, move/species/item/map lookups, JSON and text exports. |

Highest-value near-term conclusion: finish the Gen I reader foundation before attempting conversion. The safest immediate next task remains current box cache decode at `0x30C0`, because local constants already exist and both `pret/pokered`, `pokered-save-editor`, and Game Tools Collection confirm the concept.

Highest-value converter conclusion: start FireRed support with a non-destructive section reader and an unchanged round-trip writer before any semantic edit. The first semantic write experiment should target a copied FireRed save and a low-risk SaveBlock2 field such as player name, not money/items, because FireRed encrypted-value handling and security key interactions need dedicated validation.

## 2. Methodology

This pass inspected local source and external cloned repositories under `/private/tmp` and used repository search for relevant terms such as `checksum`, `save`, `party`, `box`, `pokemon`, `species`, `move`, `item`, `event`, `flag`, `map`, `trainer`, `pokedex`, `serialize`, `deserialize`, `encode`, `decode`, `convert`, `transport`, `PID`, `DV`, `IV`, `EV`, `nature`, `ability`, `WASM`, `browser`, `upload`, and `download`.

Files were opened directly before claims were recorded. The live Game Tools Collection URL was opened, but the browser tool did not return useful textual page content; the source repository is therefore the primary evidence for the live tool workflow.

The research avoided ROMs, user save files, generated save exports, and large binary assets. No working parser code was intentionally changed.

## 3. Source and Repository Inventory

| Project | URL | Inspected commit / branch | License finding | Maintenance status |
| --- | --- | --- | --- | --- |
| Current project | https://github.com/AAAMAQ/pkmn-red-save-genie | Local `main`, `e9e531b99657704e128a723de9337b13420a4925`, 2026-04-19 | Local license not evaluated in this pass | Active local source of truth |
| Pokemon Red/Blue Save Editor | https://github.com/junebug12851/pokered-save-editor | `master`, `b387f34ef20f311d5ef55ba181047357821b56d2`, 2019-03-31 | Apache-2.0 | Older but technically relevant |
| Game Tools Collection | https://github.com/RyudoSynbios/game-tools-collection | `master`, `88d5862bfa3cc581515a795c879dd8f09a0d2874`, 2026-06-18 | MIT | Recently active |
| Game Tools Collection live editor | https://game-tools-collection.com/pokemon-red-blue-and-yellow/save-editor | Live page opened; source repo used for verification | Same project license for source | Live browser tool |
| Pokemon Community Conversion Standard | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard | `latest-release`, `0186b4ebb05948bebda90ec6bc679b915b6bd893`, 2026-02-28 | No clear root license found | Recently maintained |
| Historical PCCS URL | https://github.com/GearsProgress/Pokemon-Community-Conversion-Standard | Referenced by Poke Transporter `.gitmodules`; appears moved/redirected conceptually to Striaton-Lab-Team | No separate clone inspected | Historical location |
| Poke Transporter GB | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB | `latest-release`, `34732ca884cd0b400932a6552f042e5cf6109437`, 2026-03-25 | MIT | Recently maintained |
| Pokemon Red/Blue disassembly | https://github.com/pret/pokered | `master`, `d70d99ffbd329473d96eaaf19fd97c86d2220b7f`, 2026-06-12 | No root license found in inspected clone | Actively maintained reference |
| Pokemon FireRed/LeafGreen decompilation | https://github.com/pret/pokefirered | `master`, `70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d`, 2026-06-07 | No root license found in inspected clone | Actively maintained reference |

## 4. Detailed Project Analysis

### 4.1 Current Project: pkmn-red-save-genie

Project identity:

| Field | Finding |
| --- | --- |
| Project name | `pkmn-red-save-genie` |
| Repository URL | https://github.com/AAAMAQ/pkmn-red-save-genie |
| Maintainer | `AAAMAQ` / local user project |
| Primary purpose | Gen I save research, decoding, JSON/text export, future safe editing, future Red to FireRed conversion |
| Language/build | C++ with Xcode-style project layout |
| License | Unknown / not verified in this pass |
| Inspected local commit | `e9e531b99657704e128a723de9337b13420a4925` |

Scope:

| Scope item | Status |
| --- | --- |
| Gen I save reader | Implemented for many main structures |
| Gen I save editor | Partial / future |
| Browser tool | Future |
| Pokemon converter | Future |
| Whole-save Red to FireRed converter | Future |

Architecture findings:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Layout/constants/lookups | `Pkmn Red Save Genie/HPP Files/SaveStructure.hpp`, `Pkmn Red Save Genie/CPP Files/SaveStructure.cpp` | Confirmed from code: central Gen I layout constants, text codec, species/Pokedex/item/move/map lookup tables. |
| Read-only decoder | `Pkmn Red Save Genie/HPP Files/ReadOnlyData.hpp`, `Pkmn Red Save Genie/CPP Files/ReadOnlyData.cpp` | Confirmed from code: model types include bag, stats, moves, Pokemon, boxes, and export helpers. |
| Write layer | `Pkmn Red Save Genie/HPP Files/WriteOnlyData.hpp`, `Pkmn Red Save Genie/CPP Files/WriteOnlyData.cpp` | Confirmed from code: future-facing write layer exists, repairs the main checksum for covered writes, and should remain conservative. |
| App/export | `Pkmn Red Save Genie/CPP Files/main.cpp` | Confirmed from code: loads save, validates checksums, writes `SaveGenieSummary.txt`, `PokemonBoxes.json`, and `PokemonSummary.json`. |

Current Save Genie save structure knowledge:

| Gen I area | Status | Evidence |
| --- | --- | --- |
| File size/trailing-byte tolerance | Partial | Confirmed from code and project context: warns on non-`0x8000`; future improvement is to explicitly ignore trailing bytes. |
| Main checksum | Implemented | Confirmed from code: checksum validation/export exists. |
| Bank 2/3 checksums | Implemented | Confirmed from code: validation exists. |
| Text codec | Partial | Confirmed from code: common Gen I glyphs decode; unknowns become `?`. |
| Trainer/rival/player summary | Implemented | Confirmed from code. |
| Money/coins/badges/playtime | Implemented | Confirmed from code. |
| Location | Implemented | Confirmed from code: map ID/name and X/Y exported. |
| Pokedex | Implemented | Confirmed from code. |
| Bag/PC items | Implemented | Confirmed from code. |
| Hall of Fame | Implemented | Confirmed from code. |
| Raw event flags | Partial | Confirmed from code: counts and first set flags, not named labels. |
| Party Pokemon | Implemented | Confirmed from code and project regression anchors. |
| PC boxes 1-12 | Implemented | Confirmed from code. |
| Current box cache | Partial | Confirmed from code: constants exist as `CurrentBoxCacheOff = 0x30C0` and `CurrentBoxCacheLen = 0x0462`; no `GetCurrentBoxCache()` API found. |
| Safe editing | Partial / future | Confirmed from code: write layer exists but editor surface is not mature. |

Pokemon model and conversion:

| Field group | Status |
| --- | --- |
| Species internal ID and name | Implemented |
| Pokedex number | Implemented |
| Nickname and OT name | Implemented |
| OT ID | Implemented |
| Level and EXP | Implemented |
| DVs and derived HP DV | Implemented |
| Stat Exp | Implemented |
| Party live stats/status | Implemented for party |
| Boxed live stats | Not applicable to boxed Gen I structures |
| Moves and current PP | Implemented |
| PP Ups/base max PP | Future / intentionally placeholder |
| Gen III PID/nature/IV/EV/met data | Future |

Tests and validation:

Current validation is output-driven and includes known party regression anchors in project context. A formal automated test harness was not found in the inspected current project files.

Lessons:

| Category | Lesson |
| --- | --- |
| Preserve | Do not replace verified party offsets or endian behavior with external assumptions. |
| Extend | Current box cache decode is low risk because constants and PC box parser pieces already exist. |
| Improve | Add test harness before safe editing and conversion work. |
| Avoid | Do not mix Gen I reader concerns with Gen III converter concerns inside current parser APIs. |

### 4.2 junebug12851/pokered-save-editor

Project identity:

| Field | Finding |
| --- | --- |
| Project name | `pokered-save-editor` |
| Repository URL | https://github.com/junebug12851/pokered-save-editor |
| Maintainer | `junebug12851` |
| Primary purpose | Pokemon Red/Blue save editor |
| Primary language/framework | TypeScript, Angular 7, Electron, Angular Material |
| Build system | npm scripts with Angular CLI/Electron |
| License | Apache-2.0 |
| Last inspected commit | `b387f34ef20f311d5ef55ba181047357821b56d2` |

Scope:

| Scope item | Status |
| --- | --- |
| Gen I save editor | Confirmed from documentation and code |
| Gen I save reader | Confirmed from code |
| Desktop tool | Confirmed from documentation and package metadata |
| Browser-only tool | Not found |
| Pokemon converter | Not found |
| Whole-save Gen I to Gen III converter | Not found |

Architecture:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Raw save service | `src/app/data/savefile.service.ts` | Confirmed from code: owns `Uint8Array`, exposes read/write helpers for BCD, strings, words, bits, and checksums. |
| Expanded model | `src/app/data/savefile-expanded/SaveFileExpanded.ts` | Confirmed from code: composes sections such as `Player`, `Rival`, `Storage`, `Area`, `World`, `Daycare`, and `HoF`. |
| Storage model | `src/app/data/savefile-expanded/sections/Storage.ts` | Confirmed from code: loads all boxes and overlays the current box cache. |
| Party/box Pokemon | `src/app/data/savefile-expanded/fragments/PokemonParty.ts`, `src/app/data/savefile-expanded/fragments/PokemonBox.ts` | Confirmed from code: separate party and boxed Pokemon fragments. |
| Event/world state | `src/app/data/savefile-expanded/sections/World/WorldEvents.ts` and adjacent `World` section files | Confirmed from code: reads event flags and related world-state structures. |
| Text codec | `src/app/data/text.service.ts`, `src/assets/data/text*` | Confirmed from code: table-driven Gen I text encode/decode. |
| UI shell | `src/app/*` | Confirmed from code: Angular/Electron UI, not a headless library. |

Checksum handling:

Confirmed from code in `savefile.service.ts`: `getChecksum(from, to)` subtracts bytes from `0xFF`, using an end-exclusive range. Main checksum is stored at `0x3523` and computed over `0x2598..0x3523`. Bank 2 and bank 3 all-box and individual-box checksum updates are also implemented.

Current box cache:

Confirmed from code in `Storage.ts`: current box index comes from `0x284C & 0b01111111`; bit 7 stores a changed-boxes flag. Permanent boxes are loaded from bank 2/3, then the selected current box is force-loaded from `0x30C0`. This directly supports our project rule that the Bank 1 current box cache is distinct from permanent PC Boxes 1-12 and may be fresher.

Save structure knowledge:

| Gen I area | Status | Evidence |
| --- | --- | --- |
| Player/rival/trainer data | Complete / broad | Confirmed from code in expanded sections. |
| Money/coins/badges/playtime | Complete / broad | Confirmed from code in save helpers and section model. |
| Pokedex | Present but unclear | Files indicate expanded save sections; exact field coverage not exhaustively documented in this pass. |
| Bag/PC items | Present but unclear | Storage/player sections exist; exact editing completeness not exhaustively mapped. |
| Event flags | Partial / broad | Confirmed from code: 320 bits from `0x29F3`; labels and adjacent sections exist. |
| Completed scripts/missables | Present but unclear | World section files suggest coverage; exact mapping needs a focused pass before reuse. |
| Hall of Fame | Present | Confirmed from `HoF` section composition. |
| Party | Complete / broad | Confirmed from code. |
| Current box cache | Implemented | Confirmed from code. |
| PC boxes | Implemented | Confirmed from code. |
| Text encoding | Implemented | Confirmed from code. |
| Checksums | Implemented | Confirmed from code. |
| Unused regions | Present but unclear | Unknown / not verified for exhaustive unused-region modeling. |

Pokemon model and conversion:

This project models Gen I Pokemon for editing. It is not a conversion project. Gen III fields such as PID, nature, ability, EV cap, met location, ribbons, and encrypted substructures are outside scope.

Event flags and progress:

Confirmed from code: `WorldEvents.ts` loads 320 bits from `0x29F3`. The presence of broader `World` sections is useful for future Save Genie coverage reporting. However, this pass did not verify every label or all completed-script/missable mappings, so external labels should be cross-checked against `pret/pokered` before adopting.

Player location:

Likely inference: the editor covers area/location state through its `Area` section. Exact safe-spawn semantics were not verified in this pass.

Web/UI architecture:

This is an Electron desktop application, not a browser-local web app. The UI is valuable as a rich-editor reference, but not a direct model for the future WebAssembly version.

Tests and validation:

No mature automated test suite was identified during this pass. This project is still valuable as a broad coverage checklist, not as a test strategy template.

Feature comparison with current project:

| Feature | `pokered-save-editor` | `pkmn-red-save-genie` | Gap | Possible lesson |
| --- | --- | --- | --- | --- |
| Gen I save load/edit | Broad editor | Reader/exporter plus early write layer | Save Genie lacks mature editor UI | Keep safe writer separate from reader. |
| Main checksum | Implemented | Implemented | No major gap | Compare algorithms only as confirmation, not replacement. |
| Bank box checksums | Implemented | Implemented validation | Write repair for bank edits future | Bank edit MVP must repair both all-box and individual box checksums. |
| Current box cache | Implemented and overlaid | Constants only, no getter/export | High-value small gap | Add `GetCurrentBoxCache()` using existing box parser. |
| Party | Implemented | Implemented and screenshot-validated | No near-term gap | Preserve local offsets. |
| PC boxes | Implemented | Implemented | No near-term gap | Use as conceptual cross-check. |
| World/events | Broad editor sections | Raw counts only | Event label gap | Use `pret/pokered` as authority; use editor as coverage checklist. |
| Text codec | Table-driven broad codec | Partial common codec | Fuller codec gap | Expand codec carefully with tests. |
| Hall of Fame | Present | Implemented | No major gap | Cross-check if future HoF edge cases appear. |
| Safe editing workflow | UI-driven editor | Future | Major product gap | Add narrow validated writer operations before UI. |
| Browser workflow | Electron desktop | Future | Not direct | Do not copy desktop framework into C++ core. |
| Gen III conversion | Not found | Future | Both do not solve it | Not a converter reference. |

Reuse analysis:

Apache-2.0 allows reuse with attribution and license compliance, but direct code reuse would introduce TypeScript/Angular architecture and should not replace verified C++ parser behavior. Best use is conceptual: coverage map, current-box semantics, checksum repair checklist, and editor workflow ideas.

### 4.3 Game Tools Collection Web Save Editor

Project identity:

| Field | Finding |
| --- | --- |
| Project name | Game Tools Collection |
| Repository URL | https://github.com/RyudoSynbios/game-tools-collection |
| Live URL | https://game-tools-collection.com/pokemon-red-blue-and-yellow/save-editor |
| Maintainer | `RyudoSynbios` |
| Primary purpose | Browser-based game tools and save editors |
| Primary language/framework | TypeScript, SvelteKit, Vite |
| License | MIT |
| Last inspected commit | `88d5862bfa3cc581515a795c879dd8f09a0d2874` |

Scope:

| Scope item | Status |
| --- | --- |
| Browser save editor | Confirmed from code |
| Gen I Pokemon Red/Blue/Yellow save editor | Confirmed from code |
| Client-only file manipulation | Confirmed from code for inspected route |
| Gen I to Gen III converter | Not found |
| C++/WASM core | Not found |

Architecture:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Dynamic tool route | `src/routes/[gameId]/[tool]/+page.ts`, `src/routes/[gameId]/[tool]/+page.svelte` | Confirmed from code: dynamic imports load a game/tool template and optional utility module. |
| Save schema | `src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/template.ts` | Confirmed from code: declarative `GameJson` schema defines offsets, controls, groups, and repeated Pokemon/box structures. |
| Gen I utilities | `src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/utils.ts` | Confirmed from code: handles regions, checksum validation, current box shifting, and save-specific hooks. |
| Japanese variant utility | `src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/utils/japan.ts` | Confirmed from code: separate Japanese save handling exists. |
| Fragment/resource helpers | `src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/utils/fragment.ts`, `resource.ts` | Confirmed from code: reusable fragments and resources support Pokemon/item/move/species data. |
| Checksum helper | `src/lib/utils/checksum.ts` | Confirmed from code: checksum framework used by templates. |
| Browser save/download | `src/routes/[gameId]/[tool]/+page.svelte` | Confirmed from code: local `DataView`, Blob creation, and `file-saver` download are used. |

File handling:

Confirmed from code: the route uses local file inputs/dropzone data, stores data in a browser-side `DataView`, then saves via `Blob` and `FileSaver.saveAs`. No inspected path uploads the save file to a server.

Gen I save schema highlights:

| Area | Status | Evidence |
| --- | --- | --- |
| Main checksum | Implemented | `template.ts` checksum item at `0x3523`, control `0x2598..0x3523`. |
| Bank 2/3 checksums | Implemented | `template.ts` checksum items at `0x5a4c`, `0x7a4c`. |
| Trainer name | Implemented | `0x2598`. |
| Trainer ID | Implemented | `0x2605`. |
| Rival name | Implemented | `0x25f6`. |
| Badges | Implemented | `0x2602`. |
| Playtime | Implemented | `0x2ced`, `0x2cef`, `0x2cf0`. |
| Money | Implemented | `0x25f3` BCD. |
| Coins | Implemented | `0x2850`. |
| Current box index | Implemented | `0x284c` low 7 bits. |
| Party | Implemented | Party container at `0x2f2c`, mon fragments from `0x2f34`, names and OT areas. |
| PC boxes | Implemented | Container at `0x4000`, `0x462` box size, 12 instances. |
| Current box cache | Implemented | Utility shift maps current box to Bank 1 cache using `-0xf40` for international saves. |
| Hall of Fame | Implemented | Count at `0x284e`, entries from `0x598`. |
| Japanese R/G/B/Y handling | Partial / implemented for variants | Confirmed from `japan.ts`; exact parity not exhaustively verified. |

Pokemon model and conversion:

The Gen I save editor models Pokemon fields for display/editing. It does not implement Gen III Pokemon generation, PCCS policy, or whole-save conversion.

Event flags and progress:

The schema has broad save editing patterns, but this pass did not verify a complete event flag naming model equivalent to `pret/pokered`. Treat event/progress ideas as UI/schema inspiration rather than final mapping authority.

Player location:

Likely inference: schema-driven offsets can support map/location fields if present. This pass did not establish a complete safe-spawn strategy from Game Tools.

Web/UI architecture lessons:

| Lesson | Applicability |
| --- | --- |
| Declarative save schema can drive UI controls | Useful for future Web Save Genie, not current C++ core. |
| Browser-local FileReader/DataView/Blob download workflow | Directly valuable for WebAssembly version. |
| Hook-based checksum update before save | Valuable for safe editor design. |
| Region-specific utility modules | Useful for future Red/Blue/Yellow/Japanese support separation. |
| Dynamic game/tool route registration | Useful product architecture idea if Save Genie becomes a multi-game web app. |

Tests and validation:

No focused automated test suite for Pokemon R/B/Y save editing was identified in this pass. The schema itself can inspire synthetic fixture generation.

Reuse analysis:

MIT license permits reuse with attribution. Direct reuse should be limited and deliberate because the code is Svelte/TypeScript. Strong candidates for conceptual reuse are declarative schema shape, client-only file workflow, validation hooks, and region-specific modules.

### 4.4 Pokemon Community Conversion Standard

Project identity:

| Field | Finding |
| --- | --- |
| Project name | Pokemon Community Conversion Standard |
| Repository URL | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard |
| Historical URL | https://github.com/GearsProgress/Pokemon-Community-Conversion-Standard |
| Maintainer/organization | Striaton-Lab-Team |
| Primary purpose | Gen I/II to Gen III Pokemon conversion library/standard |
| Primary language/build | C++ with Makefile |
| License | No clear root license found in inspected clone |
| Last inspected commit | `0186b4ebb05948bebda90ec6bc679b915b6bd893` |

Scope:

| Scope item | Status |
| --- | --- |
| Individual Pokemon conversion | Confirmed from documentation and code |
| Whole-save conversion | Not found |
| Save editor | Not found |
| Gen I/II box input model | Confirmed from code |
| Gen III Pokemon output model | Confirmed from code |
| FireRed save section writer | Not found |

Public API and core files:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Gen I Pokemon wrapper | `include/Gen1Pokemon.h`, `source/Gen1Pokemon.cpp` | Confirmed from code. |
| Gen II Pokemon wrapper | `include/Gen2Pokemon.h`, `source/Gen2Pokemon.cpp` | Confirmed from code. |
| Shared GB Pokemon conversion | `include/GBPokemon.h`, `source/GBPokemon.cpp` | Confirmed from code: main conversion policy lives here. |
| Gen III output model | `include/Gen3Pokemon.h`, `source/Gen3Pokemon.cpp` | Confirmed from code. |
| Box loader/converter | `include/PokeBox.h`, `source/PokeBox.cpp` | Confirmed from code: loads Gen 1/2 box data and converts all Pokemon. |
| Shared data tables | `include/pokemon_data.h` | Confirmed from code. |
| Tests/manual harness | `tests/main.cpp` | Confirmed from code: static arrays and manual conversion checks. |

Conversion methods:

| Method | Status | Evidence |
| --- | --- | --- |
| Original | Implemented | Confirmed from documentation and code: README says only Original is currently implemented, and `convertToGen3` contains this path. |
| Faithful | Future / not implemented | Confirmed from documentation. |
| Legal | Future / not implemented | Confirmed from documentation. |
| Virtual | Future / not implemented | Confirmed from documentation. |

Important conversion behavior:

| Field/policy | Finding | Label |
| --- | --- | --- |
| Input language | `PokeBox::loadData` returns early unless language is English. | Confirmed from code |
| Main API | `GBPokemon::convertToGen3(Gen3Pokemon *newPkmn, bool sanitizeMythicals)`. | Confirmed from code |
| Species | Species are generally preserved; MissingNo maps to Porygon. | Confirmed from code |
| Level | Level is copied. | Confirmed from code |
| EXP | `convertEXP()` recomputes/truncates based on current level and growth group rather than blindly preserving raw EXP. | Confirmed from code |
| Nature | Virtual Console nature rule uses `EXP % 25`. | Confirmed from code |
| PID | PID generation loops until personality satisfies constraints such as nature, gender, ability, Unown letter, and size. | Confirmed from code |
| DVs/IVs | IVs are generated through the PID/RNG path; not a simple `DV * 2` mapping in inspected implementation. | Confirmed from code |
| HP DV | Shared GB code derives HP DV from low bits of other DVs. | Confirmed from code |
| Stat Exp/EVs | Implemented path zeros EVs. | Confirmed from code |
| Moves | Illegal move filtering and move bubbling are implemented with special cases. | Confirmed from code |
| PP/PP Ups | PP totals are restored using PP Up count and Gen III base PP logic. | Confirmed from code |
| Friendship | Set to 70. | Confirmed from code |
| Met location | Set to `0xFF` fateful encounter style value. | Confirmed from code |
| Origin game | Gen I maps to FireRed; Gen II maps to HeartGold. | Confirmed from code |
| Shiny | Shiny status is detected from Gen II-style DVs and preserved/prevented via secret ID manipulation. | Confirmed from code |
| Mythicals | `sanitizeMythicals` adjusts Mew/Celebi event details. | Confirmed from code |
| Glitch Pokemon | MissingNo behavior is present; broader glitch policy was not fully verified. | Partial |

Adapter analysis for current `PokemonMon`:

| Question | Answer |
| --- | --- |
| Can current `PokemonMon` be adapted directly? | Likely yes through an adapter, but not as a direct drop-in object. PCCS expects its own Gen I/II object or raw box-style data. |
| What fields already exist? | Species, nickname, OT name, OT ID, level, EXP, moves, DVs, Stat Exp, PP current, party status/current HP for party. |
| What fields are missing or policy-dependent? | Source language/region, exact PP Ups if preserving PCCS PP logic, source game/version, conversion method selection, glitch policy, mythical sanitization policy, trainer gender/defaults, raw bytes for edge cases, legality policy, and possibly Yellow-specific friendship/Pikachu context. |
| Should PCCS be optional or integrated now? | Keep optional/reference until license is clarified and conversion policy is intentionally chosen. |
| Safest integration shape | `PokemonConversionAdapter` that maps `PokemonMon` into a neutral conversion input, with PCCS behind an optional backend. |

Save structure knowledge:

PCCS is outside whole-save scope. It does not replace a Gen I save reader or FireRed save writer. It provides Pokemon-level conversion logic only.

Tests and validation:

`tests/main.cpp` exists and contains static byte arrays and manual conversion checks. It is useful as a starting point but not sufficient as a production validation strategy for this project.

Reuse analysis:

The absence of a clear root license means direct reuse or vendoring needs legal review. Until clarified, PCCS should be treated as conceptual/reference material and possibly as an optional external dependency only after policy and license decisions.

### 4.5 Poke Transporter GB

Project identity:

| Field | Finding |
| --- | --- |
| Project name | `Poke_Transporter_GB` |
| Repository URL | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB |
| Maintainer/organization | Striaton-Lab-Team |
| Primary purpose | GBA multiboot transfer tool for moving Pokemon from Gen I/II to Gen III |
| Primary language/build | C/C++ for GBA toolchain |
| License | MIT |
| Last inspected commit | `34732ca884cd0b400932a6552f042e5cf6109437` |

Scope:

| Scope item | Status |
| --- | --- |
| Pokemon transporter | Implemented |
| Hardware/link-cable workflow | Implemented |
| Uses PCCS | Implemented |
| Gen III save sector writing | Implemented for receiving/injection use cases |
| Direct generic `.sav` file converter | Not found |
| Whole-save Gen I to FireRed conversion | Not found |

Architecture:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Documentation | `README.md`, `docs/EZ_Flash_Omega_DE.md` | Confirmed from documentation: hardware-oriented multiboot transfer tool; modifies source and target saves. |
| PCCS submodule | `.gitmodules` | Confirmed from code: points to historical `GearsProgress/Pokemon-Community-Conversion-Standard`. |
| Gen III flash/save handling | `include/flash_mem.h`, `source/flash_mem.cpp` | Confirmed from code: dual save slots, sector IDs, save indices, checksum repair, flash erase/write. |
| Save data manager | `source/save_data_manager.cpp` | Confirmed from code: high-level save operations. |
| Pokemon transfer flow | `source/pokemon_party.cpp` | Confirmed from code: loads GB box data and converts via PCCS `PokeBox`. |
| GBC link protocol | `source/gameboy_colour.cpp` | Confirmed from code: link-packet transfer and removal acknowledgement. |
| Mystery Gift / script injection | `source/mystery_gift_injector.cpp` | Confirmed from code: writes script/wonder-card style payloads and repairs checksums. |
| ROM-specific metadata | `include/rom_data.h`, `source/rom_data.cpp` | Confirmed from code: per-ROM data tables. |

Important Gen III save handling:

Confirmed from code in `flash_mem.h` and `flash_mem.cpp`: save slot A starts at `0x00000000`, save slot B starts at `0x0000E000`, section ID is at `0xFF4`, save index at `0xFFC`, and the tool reconstructs current section locations by reading section IDs and save indices. It recalculates section checksums before writing.

Transport vs converter distinction:

Confirmed from documentation and code: this project transfers Pokemon and writes receiving-side Gen III save sectors/scripts in a hardware environment. It does not reconstruct a full FireRed save from a Gen I save model, does not map Gen I story progress into FireRed story state, and does not perform full inventory/event/location conversion.

Reusable concepts:

| Category | Reuse value |
| --- | --- |
| Pokemon conversion | Uses PCCS; useful integration example. |
| Gen III save slot selection | Useful concept for future FireRed reader/writer. |
| Section checksum repair | Useful concept for future writer. |
| ROM-specific data tables | Useful architecture idea for version-specific FireRed offsets. |
| Mystery Gift/script injection | Advanced reference only; not a first writer target. |

Hardware-specific or not reusable directly:

| Category | Reason |
| --- | --- |
| GBC link protocol code | Tied to hardware transfer, not desktop save conversion. |
| Flash erase/write routines | Tied to GBA cartridge/EZ Flash environment. |
| Multiboot UI flow | Not relevant to C++ desktop/Web Save Genie core. |
| Removing Pokemon from source game through link protocol | Not part of a non-destructive save converter MVP. |

Save structure knowledge:

This project knows enough about Gen III save sections to inject/repair target save data. It is not a general FireRed save model. For FireRed section architecture, `pret/pokefirered` should remain the primary authority.

Tests and validation:

No conventional desktop unit test suite was identified. Hardware testing assumptions dominate.

Reuse analysis:

MIT license permits reuse with attribution. Practical reuse should be conceptual for save-slot/checksum logic and optional for small algorithms after review. Hardware code should not be copied into the desktop core.

### 4.6 pret/pokered

Project identity:

| Field | Finding |
| --- | --- |
| Project name | `pret/pokered` |
| Repository URL | https://github.com/pret/pokered |
| Maintainer/organization | pret |
| Primary purpose | Pokemon Red/Blue disassembly |
| Primary language/build | RGBDS assembly |
| License | No root license found in inspected clone |
| Last inspected commit | `d70d99ffbd329473d96eaaf19fd97c86d2220b7f` |

Scope:

| Scope item | Status |
| --- | --- |
| Gen I disassembly | Implemented/reference |
| Save routine reference | Reference only |
| Event/map/item/species constants | Reference only |
| Save editor | Not applicable |
| Whole-save converter | Not applicable |

Architecture and authoritative files:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| SRAM layout | `ram/sram.asm` | Confirmed from code: defines Hall of Fame, main data, party data, current box data, checksums, boxes 1-12, bank checksums. |
| WRAM layout | `ram/wram.asm` | Confirmed from code: defines player data, party, Pokedex, bag, rival, location, box items, event flags, current map script, box count. |
| Save/load routines | `engine/menus/save.asm` | Confirmed from code: contains `TryLoadSaveFile`, `LoadMainData`, `SaveMainData`, `SaveCurrentBoxData`, `SavePartyAndDexData`, `CalcCheckSum`, and box-copy routines. |
| Event constants | `constants/event_constants.asm` | Confirmed from code: event constants use `const_def`, `const_next`, and `const_skip`; contains `EVENT_BEAT_*`. |
| Text charmap | `constants/charmap.asm` | Confirmed from code. |
| Items/species/moves/maps | `constants/item_constants.asm`, `constants/pokemon_constants.asm`, `constants/move_constants.asm`, `constants/map_constants.asm` | Confirmed from code. |

Key save facts confirmed:

| Fact | Evidence |
| --- | --- |
| Current box cache is saved separately in Bank 1 | `sCurBoxData` in `ram/sram.asm`; `SaveCurrentBoxData` in `engine/menus/save.asm`. |
| Current box is copied between SRAM and WRAM during box changes | `ChangeBox`, `GetBoxSRAMLocation`, `CopyBoxToOrFromSRAM` in `save.asm`. |
| Main checksum is complement of summed bytes | `CalcCheckSum` in `save.asm` sums and then complements. |
| Stored PC boxes have individual and all-box checksums | `ram/sram.asm` symbols and `CalcIndividualBoxCheckSums`. |

Event flags and progress:

`constants/event_constants.asm` is the key source for Gen I event names and trainer-defeated flags. Because it uses macro stepping and skips, event-name mapping must be generated by parsing the macro semantics, not by copying names into a naive contiguous table.

Player location:

`ram/wram.asm` includes `wCurMap`, `wYCoord`, and `wXCoord`. Map metadata and warps are in `data/maps` and `constants/map_constants.asm`. This confirms Save Genie can start with Gen I map ID/X/Y and later map it to a known-good FireRed fallback location.

Tests and validation:

This is not a test suite for Save Genie. It is an authoritative reference for symbols and behavior. Any extracted mapping should be validated against real or synthetic save outputs.

Reuse analysis:

No clear root license was found in the inspected clone. Treat as reference-only unless licensing is clarified. It is still the best authority for offsets, symbols, and behavior.

### 4.7 pret/pokefirered

Project identity:

| Field | Finding |
| --- | --- |
| Project name | `pret/pokefirered` |
| Repository URL | https://github.com/pret/pokefirered |
| Maintainer/organization | pret |
| Primary purpose | Pokemon FireRed/LeafGreen decompilation |
| Primary language/build | C, assembly, agbcc-style toolchain |
| License | No root license found in inspected clone |
| Last inspected commit | `70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d` |

Scope:

| Scope item | Status |
| --- | --- |
| FireRed/LeafGreen decompilation | Implemented/reference |
| FireRed save format reference | Reference only |
| FireRed flags/vars/maps reference | Reference only |
| Save editor | Not applicable |
| Gen I to Gen III converter | Not applicable |

Architecture and authoritative files:

| Area | Inspected paths | Claim |
| --- | --- | --- |
| Save sections | `src/save.c` | Confirmed from code: save slots, sectors, section IDs, signatures, counters, checksums. |
| Save block structs | `include/global.h` | Confirmed from code: `SaveBlock1`, `SaveBlock2`, `PokemonStorage`, player party, flags, vars, item data, locations. |
| Serialized save helpers | `include/load_save.h` | Confirmed from code: save/load helpers for party, bag, encryption key handling. |
| Event data | `src/event_data.c` | Confirmed from code: flag/var helpers and National Dex enabling. |
| Pokemon structs | `include/pokemon.h` | Confirmed from code: `BoxPokemon`, encrypted/shuffled substructs, party `Pokemon`. |
| Flags | `include/constants/flags.h` | Confirmed from code. |
| Vars | `include/constants/vars.h` | Confirmed from code. |
| Maps | `include/constants/maps.h`, `data/maps/map_groups.json`, `data/maps/**/map.json`, `data/maps/**/scripts.inc` | Confirmed from code/search. |
| PC storage | `src/pokemon_storage_system.c` | Confirmed from code/search. |
| Pokedex | `src/pokedex.c` | Confirmed from code/search. |

Save structure knowledge:

| Gen III area | Status | Evidence |
| --- | --- | --- |
| Save block selection | Reference only / authoritative | `src/save.c`. |
| Section rotation | Reference only / authoritative | `src/save.c`. |
| Section IDs/signature/save index | Reference only / authoritative | `src/save.c`. |
| Section checksums | Reference only / authoritative | `src/save.c`. |
| Trainer info | Reference only / authoritative | `include/global.h` `SaveBlock2`. |
| Money/items | Reference only / authoritative | `include/global.h` `SaveBlock1`; encryption handling needs focused pass. |
| Party | Reference only / authoritative | `include/global.h`, `include/pokemon.h`. |
| PC storage | Reference only / authoritative | `PokemonStorage`, `src/pokemon_storage_system.c`. |
| Pokedex | Reference only / authoritative | `SaveBlock2`, `src/pokedex.c`. |
| Event flags | Reference only / authoritative | `SaveBlock1.flags`, `include/constants/flags.h`, `src/event_data.c`. |
| Variables | Reference only / authoritative | `SaveBlock1.vars`, `include/constants/vars.h`. |
| Map/location | Reference only / authoritative | `SaveBlock1.pos`, map constants/data. |
| Security key | Reference only / authoritative | `SaveBlock2.encryptionKey`, load/save helpers. |
| Encrypted Pokemon structures | Reference only / authoritative | `include/pokemon.h`. |

Sevii Islands concern:

Confirmed from code/search: Sevii state touches multiple systems: `FLAG_SYS_CAN_LINK_WITH_RS`, `FLAG_SYS_NATIONAL_DEX`, `VAR_NATIONAL_DEX`, map scene vars for Sevii locations, seagallop destination scripts, object hide flags, Elite Four/Hall of Fame scripts, and National Dex state. A converted post-League save can become inconsistent if it grants badges/League completion/National Dex without matching Sevii and Network Machine progression state.

Safe policy implication:

Initial whole-save conversion should use a conservative known-good FireRed state or a clearly documented safe-state policy rather than attempting perfect Gen I story preservation. Story-preserving conversion can come later after flags/vars are mapped and tested.

Reuse analysis:

No clear root license was found in the inspected clone. Treat as reference-only until licensing is clarified. It is the primary technical map for FireRed save reader/writer implementation.

## 5. License and Reuse Analysis

| Project | License found | Direct reuse | Best current use |
| --- | --- | --- | --- |
| Current project | Unknown / not verified | Not applicable | Source of truth. |
| `pokered-save-editor` | Apache-2.0 | Possible with attribution and compliance | Conceptual reference for broad Gen I editor coverage, current box cache, checksum repair, UI. |
| Game Tools Collection | MIT | Possible with attribution | Conceptual and possible technical reference for browser-local workflow and declarative schemas. |
| PCCS | No clear root license found | Legal review needed | Reference/optional dependency candidate only. |
| Poke Transporter GB | MIT | Possible with attribution | Conceptual reference for PCCS integration and Gen III save-sector handling; hardware parts should not be copied. |
| `pret/pokered` | No root license found in inspected clone | Reference-only until clarified | Authoritative symbols and behavior reference. |
| `pret/pokefirered` | No root license found in inspected clone | Reference-only until clarified | Authoritative FireRed symbols and behavior reference. |

## 6. Feature Comparison Matrix

Status values: Implemented, Partial, Reference only, Not applicable, Not found, Future.

| Feature | pkmn-red-save-genie | pokered-save-editor | Game Tools Collection | PCCS | Poke Transporter GB | pret/pokered | pret/pokefirered |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Gen I file validation | Partial | Implemented | Implemented | Not applicable | Partial | Reference only | Not applicable |
| Gen I checksum | Implemented | Implemented | Implemented | Not applicable | Partial | Reference only | Not applicable |
| Gen I text codec | Partial | Implemented | Implemented | Partial | Partial | Reference only | Not applicable |
| Trainer summary | Implemented | Implemented | Implemented | Not applicable | Not applicable | Reference only | Not applicable |
| Pokedex | Implemented | Present but unclear | Implemented | Not applicable | Not applicable | Reference only | Reference only |
| Inventory | Implemented | Present but unclear | Implemented | Not applicable | Not applicable | Reference only | Reference only |
| Hall of Fame | Implemented | Present | Implemented | Not applicable | Not applicable | Reference only | Reference only |
| Party decode | Implemented | Implemented | Implemented | Partial | Partial | Reference only | Reference only |
| PC boxes | Implemented | Implemented | Implemented | Partial | Partial | Reference only | Reference only |
| Current box cache | Future / constants exist | Implemented | Implemented | Not applicable | Not applicable | Reference only | Not applicable |
| Event flag names | Future | Partial | Present but unclear | Not applicable | Not applicable | Reference only | Reference only |
| Trainer defeated flags | Future | Present but unclear | Present but unclear | Not applicable | Not applicable | Reference only | Reference only |
| Safe editing | Partial | Implemented | Implemented | Not applicable | Partial | Not applicable | Not applicable |
| Browser-local editing | Future | Not found | Implemented | Not applicable | Not applicable | Not applicable | Not applicable |
| WebAssembly compatibility | Future | Not found | Not found | Possible | Not applicable | Not applicable | Not applicable |
| Gen III save reading | Future | Not applicable | Not applicable | Not applicable | Partial | Not applicable | Reference only |
| Gen III save writing | Future | Not applicable | Not applicable | Not applicable | Partial | Not applicable | Reference only |
| Gen III Pokemon insertion | Future | Not applicable | Not applicable | Produces Pokemon objects | Implemented for transporter flow | Not applicable | Reference only |
| Pokemon conversion standard | Future | Not applicable | Not applicable | Implemented / partial methods | Implemented via PCCS | Not applicable | Not applicable |
| Whole-save Gen I to Gen III conversion | Future | Not found | Not found | Not found | Not found | Not applicable | Not applicable |
| Location mapping | Future | Partial | Present but unclear | Not applicable | Not applicable | Reference only | Reference only |
| Story progression mapping | Future | Partial | Present but unclear | Not applicable | Not applicable | Reference only | Reference only |
| Sevii/postgame consistency | Future | Not applicable | Not applicable | Not applicable | Partial receiving-side concern | Not applicable | Reference only |

Notes:

| Note | Explanation |
| --- | --- |
| `pokered-save-editor` breadth | It appears to have the broadest Gen I editor coverage among inspected tools, but `pret/pokered` remains the authority for final event and save-symbol mapping. |
| Game Tools browser value | Its most important contribution is architecture and workflow for client-local editing, not conversion logic. |
| PCCS status | It is Pokemon-level conversion only; README documents multiple modes, but inspected code currently implements the Original path. |
| Poke Transporter GB status | It writes Gen III save sectors for transfer/injection, but it is not a generic `.sav` whole-save converter. |

## 7. pret/pokered Research Source Map

| Research topic | Authoritative pokered path | Important symbols/macros | Relation to Save Genie | Priority |
| --- | --- | --- | --- | --- |
| SRAM layout | `ram/sram.asm` | `sHallOfFame`, `sGameData`, `sPartyData`, `sCurBoxData`, `sBox1`..`sBox12`, checksum symbols | Confirms bank layout and current box cache | Critical |
| WRAM layout | `ram/wram.asm` | `wPlayerName`, `wPartyCount`, `wPokedexOwned`, `wPokedexSeen`, `wNumBagItems`, `wRivalName`, `wCurMap`, `wXCoord`, `wYCoord`, `wEventFlags`, `wBoxCount` | Maps saved WRAM fields to Save Genie models | Critical |
| Save/load routines | `engine/menus/save.asm` | `TryLoadSaveFile`, `LoadMainData`, `SaveMainData`, `SaveCurrentBoxData`, `SavePartyAndDexData`, `CalcCheckSum` | Confirms checksum and copy behavior | Critical |
| SRAM banking | `engine/menus/save.asm`, `home/*.asm` searches | `BankswitchSRAM`, `EnableSRAM`, `DisableSRAM` | Needed for exact bank behavior and coverage report | High |
| Main checksum | `engine/menus/save.asm` | `CalcCheckSum` | Confirms complement checksum | Critical |
| Party | `ram/wram.asm`, `ram/sram.asm`, Pokemon struct definitions by symbols | `wPartyDataStart`, `wPartyCount` | Confirms party block source | High |
| Current PC box | `ram/sram.asm`, `engine/menus/save.asm` | `sCurBoxData`, `wBoxDataStart`, `SaveCurrentBoxData` | Direct source for next task | Critical |
| Stored PC boxes | `ram/sram.asm`, `engine/menus/save.asm` | `sBox1`..`sBox12`, `GetBoxSRAMLocation`, `CopyBoxToOrFromSRAM` | Confirms PC box offsets/checksums | Critical |
| Player/rival/trainer data | `ram/wram.asm` | `wPlayerName`, `wRivalName`, `wPlayerID` | Confirms trainer summary fields | High |
| Pokedex | `ram/wram.asm` | `wPokedexOwned`, `wPokedexSeen` | Confirms bitset ranges | High |
| Bag/PC items | `ram/wram.asm` | `wNumBagItems`, `wNumBoxItems` | Confirms inventory structures | High |
| Event flags | `constants/event_constants.asm`, `ram/wram.asm` | `EVENT_*`, `wEventFlags`, `const_def`, `const_next`, `const_skip` | Needed for named event export | Critical |
| Completed scripts | `ram/wram.asm`, map script files | `wCurMapScript`, map script labels | Needed for story state interpretation | Medium |
| Defeated trainers | `constants/event_constants.asm` | `EVENT_BEAT_*` | Needed for trainer flag labels | Critical |
| Missable objects | `constants/event_constants.asm`, `data/maps/toggleable_objects.asm` | missable/toggle events | Needed for world state coverage | High |
| Maps | `constants/map_constants.asm`, `data/maps/*` | map constants, map headers | Needed for location mapping | High |
| Warps | `data/maps/objects/*.asm`, `data/maps/special_warps.asm` | `warp_event`, special warp tables | Needed for future safe location conversion | Medium |
| Text encoding | `constants/charmap.asm` | charmap definitions | Needed for fuller Gen I codec | High |
| Species/moves/items | `constants/pokemon_constants.asm`, `constants/move_constants.asm`, `constants/item_constants.asm` | constants | Lookup source for table audit | Medium |

Research strategy for Gen I events: build a parser for `const_def`, `const`, `const_next`, and `const_skip`, generate event index/name pairs, then validate against known set bits in real saves and existing raw event counts. Do not hand-number these events casually.

## 8. pret/pokefirered Research Source Map

| Research topic | Authoritative pokefirered path | Important symbols/macros | Relation to future converter | Priority |
| --- | --- | --- | --- | --- |
| Save blocks/sections | `src/save.c` | `sSaveSlotLayout`, sector IDs, save slots | Foundation for FireRed reader/writer | Critical |
| Section IDs/signature/index | `src/save.c` | section `id`, `signature`, `counter` | Needed to select latest valid save | Critical |
| Section checksum | `src/save.c` | `CalculateChecksum`, `HandleWriteSector` | Required for any FireRed write | Critical |
| Security key | `include/global.h`, `include/load_save.h` | `SaveBlock2.encryptionKey`, encryption key helpers | Needed before editing encrypted fields | Critical |
| Trainer data | `include/global.h` | `SaveBlock2.playerName`, gender, trainerId, playTime | Safest initial semantic write target | High |
| Money/items | `include/global.h`, `include/load_save.h` | `SaveBlock1.money`, item pockets, encryption helpers | Important later; must handle key behavior | High |
| Party | `include/global.h`, `include/pokemon.h` | `playerParty`, `Pokemon` | Needed for Gen III Pokemon insertion | Critical |
| Pokemon serialization/encryption | `include/pokemon.h`, Pokemon source files | `BoxPokemon`, substructs, personality, checksum | Required for conversion correctness | Critical |
| PC storage | `include/global.h`, `src/pokemon_storage_system.c` | `PokemonStorage`, `SetBoxMonAt`, storage helpers | Needed for converted boxes | Critical |
| Pokedex | `include/global.h`, `src/pokedex.c` | `Pokedex`, seen/owned flags, National Dex fields | Needed for conversion | High |
| Flags | `include/constants/flags.h`, `src/event_data.c` | `FLAG_*`, `FlagSet`, `FlagGet` | Story/progress mapping | Critical |
| Variables | `include/constants/vars.h`, `src/event_data.c` | `VAR_*`, `VarSet`, `VarGet` | Story/progress mapping | Critical |
| Defeated trainers | `include/global.h`, `src/vs_seeker.c`, scripts | `trainerRematches`, trainer flags/scripts | Needed for progress conversion | High |
| Maps | `include/constants/maps.h`, `data/maps/map_groups.json`, `data/maps/**/map.json` | map constants/groups | Location conversion map | High |
| Warps | `data/maps/**/map.json`, `data/maps/**/scripts.inc` | warp definitions, scripts | Safe spawn strategy | High |
| Player coordinates | `include/global.h` | `SaveBlock1.pos`, warp fields | FireRed location writer | Critical |
| Postgame | `data/scripts/hall_of_fame.inc`, `src/hall_of_fame.c`, flags/vars | Hall of Fame and postgame flags | Prevent inconsistent postgame state | High |
| Sevii Islands | `data/scripts/seagallop.inc`, `data/maps/*Island*/scripts.inc`, `src/field_specials.c` | Sevii map scene vars, `GetUnlockedSeviiAreas`, `FLAG_SYS_CAN_LINK_WITH_RS` | Major consistency risk | Critical |
| Elite Four completion | Hall of Fame scripts and flags | League/postgame transitions | Needs conservative policy | High |
| National Pokedex | `src/event_data.c`, `include/constants/flags.h`, `include/constants/vars.h` | `EnableNationalPokedex`, `FLAG_SYS_NATIONAL_DEX`, `VAR_NATIONAL_DEX`, `nationalMagic` | Must synchronize with postgame policy | Critical |

Research strategy for FireRed: implement a read-only save container first, verify latest-slot selection and section checksums on test fixtures, then perform unchanged round-trip writes before any field edit.

## 9. Gen I Save Genie Gap Analysis

| Gap | Rank | Why it matters | Suggested validation |
| --- | --- | --- | --- |
| Current box cache getter/export | Critical | The current in-game box can be fresher than stored box copy; constants already exist. | Compare count/species/nicknames from cache with permanent current-box copy on known save. |
| Event flag names | High | Raw flags are useful but hard to interpret; needed for story conversion. | Macro-parse `pret/pokered` events and compare raw set indices. |
| Defeated trainer labels | High | Trainer progress is important for story mapping. | Validate `EVENT_BEAT_*` labels on known saves or controlled save diffs. |
| Formal test harness | High | Conversion and writing require confidence before edits. | Synthetic byte buffers, checksum vectors, golden JSON snapshots. |
| Fuller text codec | Medium | Hacked/modded names show unknown glyphs; broader export quality. | Round-trip known charmap bytes from `pret/pokered`. |
| Trailing-byte behavior | Medium | Some saves are `0x802C`; current behavior warns. | Test first `0x8000` parse with trailing bytes ignored. |
| Safe editor MVP | High | Needed for product direction, but must not destabilize reader. | Copy-only writes, range validation, checksum repair, before/after decode. |
| Save coverage report | Medium | Helps plan remaining fields and unknown ranges. | Generate static coverage table from constants and decoded ranges. |
| Version/region support | Medium | Red/Blue/Yellow/Japanese differ. | Separate layout profiles and synthetic fixtures. |

## 10. Gen III Reader/Writer Gap Analysis

| Gap | Rank | Why it matters | Suggested validation |
| --- | --- | --- | --- |
| FireRed save container reader | Critical | Must select latest slot and reconstruct sections before writing. | Known valid saves or synthetic sector fixtures with save index ordering. |
| Section checksum verification | Critical | Every FireRed write depends on section checksums. | Independent checksum vectors and unchanged round-trip. |
| Section writer/round-trip | Critical | Safest first write capability. | Read and rewrite unchanged copy; byte-diff expected metadata only if intentionally changed. |
| SaveBlock1/2/PokemonStorage models | High | Needed for trainer, location, party, PC, Pokedex, flags. | Parse selected fields and compare to emulator screenshots or controlled diffs. |
| Security key/encrypted values | High | Money/items and other encrypted data can corrupt if mishandled. | Start with non-encrypted fields; add dedicated key tests later. |
| Gen III Pokemon codec | Critical | Converted Pokemon require correct encryption/shuffle/checksum. | Synthetic Pokemon encode/decode round trips and known legal specimens. |
| Flags/vars map | Critical | Story and Sevii state depend on them. | Controlled save diffs by toggling story events. |
| Location writer | High | Invalid map/warp/coords can softlock. | Known-good spawn table and emulator boot tests. |

## 11. Pokemon Conversion Integration Analysis

| Gap | Rank | Why it matters | Direction |
| --- | --- | --- | --- |
| Neutral conversion input model | High | Avoid coupling Save Genie directly to PCCS or Gen III writer. | Add later as `PokemonConversionAdapter` input/output DTO. |
| PCCS license clarity | Critical | Direct reuse is blocked without license confidence. | Ask maintainers or inspect history/releases for license. |
| Conversion policy selection | High | Faithfulness vs legality changes output. | Define `ConversionPolicy` before implementation. |
| Missing source metadata | Medium | Source language/version and PP Ups affect conversion. | Extend model only when policy requires it. |
| Deterministic output | High | Reproducibility matters for debugging and trust. | Use deterministic seed from source mon + policy; record in `ConversionReport`. |
| Glitch Pokemon policy | Medium | Gen I saves may contain invalid species/moves. | Start with reject/report or safe placeholder policy. |
| EV/IV/PID/nature rules | High | Determines legality and user expectations. | Compare PCCS, Poke Transporter, and chosen policy. |

## 12. Whole-Save Converter Gap Analysis

| Gap | Rank | Why it matters | Direction |
| --- | --- | --- | --- |
| Badge/story mapping | High | Badges alone do not describe FireRed progression. | Conservative safe-state policy first. |
| Event mapping | Critical | Thousands of flags/vars across games are not one-to-one. | Research and map incrementally. |
| Defeated trainer mapping | High | Affects world state, rematches, and player continuity. | Start with named labels, not conversion. |
| Inventory conversion | Medium | Gen I items do not all map cleanly to Gen III. | Map common items; report unsupported items. |
| Pokedex conversion | High | Must synchronize owned/seen/National Dex policy. | Decide whether converted Pokemon imply dex state. |
| Location fallback | High | Exact coordinate matching is unnecessary for MVP but safe spawn is required. | Lookup table to known-good FireRed positions. |
| Sevii consistency | Critical | FRLG postgame and islands can conflict with converted League state. | Avoid story preservation initially or use a tested template state. |
| Conversion report | High | Users need to know what was preserved, changed, generated, or dropped. | Add `ConversionReport` as a first-class artifact. |

## 13. Architecture Recommendations

| Module | Need | Inspired by | Now/later | Refactor risk |
| --- | --- | --- | --- | --- |
| `Gen1Layout` | Keep raw offsets/checksum constants centralized. | Current project, `pret/pokered` | Already exists conceptually | Low |
| `Gen1TextCodec` | Decode/encode Gen I text safely. | Current project, `pokered-save-editor`, `pret/pokered` | Existing, expand later | Low |
| `Gen1SaveReader` | Clearer read-only boundary around `ReadOnlyData`. | Current project | Later if code grows | Medium |
| `Gen1SaveWriter` | Safe copy-edit API with validation/checksum repair. | Current `WriteOnlyData`, web editor patterns | Near-term | Medium |
| `Gen1SaveModel` | Stable intermediate data model for converter. | Current structs | Near-term after reader stabilizes | Medium |
| `Gen3SaveContainer` | Parse FireRed two-slot/sectioned save. | `pret/pokefirered`, Poke Transporter GB | Mid-term | Low if new module |
| `Gen3SectionReader` | Verify and expose section data. | `pret/pokefirered` | Mid-term | Low if new module |
| `Gen3SectionWriter` | Rewrite sections and repair checksums. | `pret/pokefirered`, Poke Transporter GB | Mid-term | Medium |
| `Gen3PokemonCodec` | Encode/decode encrypted Gen III Pokemon. | `pret/pokefirered`, PCCS | Mid-term | Medium |
| `FireRedSaveModel` | Target-side trainer, party, PC, Pokedex, flags, vars, location. | `pret/pokefirered` | Mid-term | Medium |
| `PokemonConversionAdapter` | Bridge `PokemonMon` to chosen conversion policy/PCCS. | PCCS, Poke Transporter GB | Long-term after policy | Low if isolated |
| `ProgressConversionMap` | Map badges/events/trainers/story state. | pret repos | Long-term | High |
| `LocationConversionMap` | Safe Gen I map to FireRed spawn table. | Expert advice, pret maps | Long-term, can start small | Low |
| `ConversionPolicy` | Make legality/faithfulness/defaults explicit. | PCCS modes | Before conversion | Low |
| `ConversionReport` | Explain preserved/generated/dropped fields. | Converter risk analysis | Before conversion | Low |
| `ValidationReport` | Centralize warnings, checksum status, unsupported data. | Save editor workflows | Near-term | Low |

Recommendation: do not perform a major architecture refactor now. Continue with surgical add-only reader improvements, then introduce new modules when FireRed work begins.

## 14. Testing Recommendations

| Test idea | Purpose | Copyright-safe? |
| --- | --- | --- |
| Synthetic `0x8000` Gen I save buffers | Validate offsets, counts, clamping, and checksums without real saves. | Yes |
| Handcrafted party fixture | Lock party struct parsing and regression anchors. | Yes |
| Handcrafted PC box fixture | Validate 20-slot box parsing and invalid count clamping. | Yes |
| Current box cache fixture | Verify `0x30C0` cache decode without changing PC boxes. | Yes |
| Checksum vectors | Validate main and bank checksum algorithms. | Yes |
| Trailing-byte fixture | Confirm `0x802C`-style files parse first `0x8000` safely. | Yes |
| JSON golden snapshots | Catch accidental schema changes. | Yes |
| Event macro parser tests | Validate `const_skip`/`const_next` handling. | Yes |
| FireRed synthetic section container | Test section IDs, save indices, and checksums. | Yes |
| FireRed unchanged round-trip test | Prove writer safety before semantic edits. | Yes |
| Gen III Pokemon encode/decode round-trip | Validate personality, substructure order, checksum, encryption. | Yes |
| Conversion report snapshots | Ensure generated/dropped fields are transparent. | Yes |

## 15. Prioritized Roadmap

### Immediate

| Task | Goal | Why now | Dependencies | Likely files/modules | Reference | Validation | Risk |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Decode current box cache | Add `GetCurrentBoxCache()` and optional full JSON export. | Low-risk reader extension; constants exist. | Existing PC box parser. | `ReadOnlyData`, `main.cpp` JSON helper only when implementing later. | Current project, `pret/pokered`, `pokered-save-editor`, Game Tools. | Compare count and species; do not alter party/PC boxes/checksums. | Low |
| Add basic tests/fixtures | Lock current reader behavior before edits. | Prevent regressions in verified party/box decode. | Test harness decision. | New test files only. | Current regression anchors. | Automated build/test. | Medium |
| Improve compact summary JSON | Add selected trainer/dex/cache counts if requested. | Useful dashboard data, low risk. | Current models. | `main.cpp`. | Current project. | JSON validity/golden output. | Low |
| Save coverage report | Document decoded/unknown save ranges. | Helps future research. | Current constants. | New docs/tool later. | `pret/pokered`, current project. | Manual review. | Low |

### Near-term

| Task | Goal | Why now | Dependencies | Likely files/modules | Reference | Validation | Risk |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Event/trainer flag labels | Generate event index to name table. | Needed for progress understanding. | Macro parser for `pret/pokered` constants. | `SaveStructure`, `ReadOnlyData` or generated data. | `constants/event_constants.asm`. | Known set flags and controlled diffs. | Medium |
| Fuller Gen I text codec | Reduce `?` for known glyphs. | Better exports and future safe name editing. | Charmap audit. | `Gen1TextCodec`. | `pret/pokered`, editor text tables. | Round-trip char tests. | Medium |
| Safe editor MVP | Copy-save edits for names/money/coins/badges/items. | Product direction after reader stabilizes. | Tests and checksum repair. | `WriteOnlyData`. | `pokered-save-editor`, Game Tools. | Before/after decode and checksum tests. | Medium |
| Explicit trailing-byte policy | Parse first `0x8000`, warn ignored bytes. | Real saves may be larger. | File loader update. | `main.cpp` or loader utility. | Current project context. | `0x802C` synthetic fixture. | Low |

### Mid-term

| Task | Goal | Why now | Dependencies | Likely files/modules | Reference | Validation | Risk |
| --- | --- | --- | --- | --- | --- | --- | --- |
| FireRed save container reader | Select latest slot and expose sections. | Required before any converter. | None beyond research. | `Gen3SaveContainer`, `Gen3SectionReader`. | `pret/pokefirered`, Poke Transporter GB. | Synthetic section tests. | Medium |
| FireRed unchanged round-trip writer | Rewrite copied save without semantic changes. | Safest first writer experiment. | Container reader/checksums. | `Gen3SectionWriter`. | `src/save.c`. | Byte diff and checksum verification. | Medium |
| First FireRed semantic write | Edit copied SaveBlock2 player name only. | Low-risk proof of write path. | Round-trip writer. | `FireRedSaveModel`, writer. | `include/global.h`. | Emulator/load verification if possible. | Medium |
| Gen III Pokemon codec | Encode/decode BoxPokemon/Pokemon. | Required for converted party/boxes. | Section writer. | `Gen3PokemonCodec`. | `include/pokemon.h`, PCCS. | Round-trip fixtures. | High |

### Long-term

| Task | Goal | Why now | Dependencies | Likely files/modules | Reference | Validation | Risk |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Pokemon conversion adapter | Convert `PokemonMon` to Gen III Pokemon. | Core converter milestone. | Gen III Pokemon codec and policy. | `PokemonConversionAdapter`, `ConversionPolicy`. | PCCS, Poke Transporter GB. | Deterministic conversion tests. | High |
| Location fallback map | Map Gen I locations to safe FireRed spawns. | Prevent invalid loads. | FireRed model. | `LocationConversionMap`. | pret maps. | Emulator boot tests. | Medium |
| Conservative progress policy | Avoid broken story states. | Full preservation is hard. | Flags/vars research. | `ProgressConversionMap`, `ConversionReport`. | pret repos. | Scenario tests. | High |
| Whole-save conversion MVP | Produce valid FireRed save from Gen I model. | Long-term goal. | All above. | Converter pipeline modules. | All sources. | Emulator load and report review. | Critical |

## 16. Open Research Questions and Answers

| Question | Answer |
| --- | --- |
| Does any linked project already perform whole-save Gen I to FireRed conversion? | None of the analyzed projects was found to implement full whole-save Gen I to FireRed conversion. |
| Which project has the broadest Gen I save coverage? | Among editor tools, `pokered-save-editor` appears broadest. For authoritative behavior, `pret/pokered` is the key reference. |
| Which project has the most useful browser-local file workflow? | Game Tools Collection, confirmed from Svelte source using local data views, Blob creation, and download flow. |
| Can PCCS accept current `PokemonMon` data with an adapter? | Likely yes, but not directly as-is. An adapter should construct PCCS-compatible input or a neutral conversion DTO. |
| What fields are missing from `PokemonMon` for PCCS? | Source language/region, source game/version, PP Ups if preserving PP policy, conversion method, glitch/mythical policy, legality policy, trainer defaults, and raw bytes for edge cases. |
| Does Poke Transporter GB write Gen III save files directly or use another transfer mechanism? | It uses hardware/link transfer and writes Gen III save sectors/scripts on cartridge/flash. It is not a generic desktop `.sav` converter. |
| What parts of Poke Transporter GB are reusable outside its original environment? | Save-slot selection concepts, section checksum concepts, PCCS integration pattern, ROM metadata table idea. Hardware link and flash routines are not directly reusable. |
| Where exactly are Gen I trainer-defeated flags defined? | `pret/pokered/constants/event_constants.asm`, especially `EVENT_BEAT_*`, with macro semantics that must be parsed. |
| Where exactly are FireRed trainer/event flags defined? | `pret/pokefirered/include/constants/flags.h`, with usage in `data/maps/**/scripts.inc`; rematch state also touches `SaveBlock1.trainerRematches` and related VS Seeker code. |
| What is the safest first FireRed write experiment? | Read a copied save, select latest sections, rewrite unchanged with valid checksums, then edit a low-risk SaveBlock2 field such as player name. |
| What FireRed state is required to avoid Sevii Islands inconsistencies? | National Dex fields/flags/vars, `FLAG_SYS_CAN_LINK_WITH_RS`, Sevii map scene vars, seagallop availability, object hide flags, Elite Four/Hall of Fame state, and safe location/warp fields must be coherent. |
| Should initial whole-save conversion preserve story progress or use a conservative safe-state policy? | Use a conservative safe-state policy first. Story-preserving conversion should come after flag/var mapping and save-diff validation. |
| What tests can be created without committing copyrighted saves? | Synthetic byte buffers, handcrafted party/box fixtures, checksum vectors, JSON snapshots, malformed/trailing-byte fixtures, event macro parser tests, FireRed synthetic sections, and encode/decode round trips. |
| Which external code is reusable under its license? | `pokered-save-editor` is Apache-2.0, Game Tools Collection is MIT, Poke Transporter GB is MIT. Reuse still needs attribution and compatibility review. |
| Which ideas should remain reference-only? | pret source until license is clarified, PCCS until license is clarified, hardware-specific transporter code, and any external offset that conflicts with verified local parser behavior. |

## 17. Exact Source Links

| Source | Link |
| --- | --- |
| Current public repo | https://github.com/AAAMAQ/pkmn-red-save-genie |
| `pokered-save-editor` inspected tree | https://github.com/junebug12851/pokered-save-editor/tree/b387f34ef20f311d5ef55ba181047357821b56d2 |
| `pokered-save-editor` save service | https://github.com/junebug12851/pokered-save-editor/blob/b387f34ef20f311d5ef55ba181047357821b56d2/src/app/data/savefile.service.ts |
| `pokered-save-editor` storage section | https://github.com/junebug12851/pokered-save-editor/blob/b387f34ef20f311d5ef55ba181047357821b56d2/src/app/data/savefile-expanded/sections/Storage.ts |
| Game Tools Collection inspected tree | https://github.com/RyudoSynbios/game-tools-collection/tree/88d5862bfa3cc581515a795c879dd8f09a0d2874 |
| Game Tools live editor | https://game-tools-collection.com/pokemon-red-blue-and-yellow/save-editor |
| Game Tools Gen I template | https://github.com/RyudoSynbios/game-tools-collection/blob/88d5862bfa3cc581515a795c879dd8f09a0d2874/src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/template.ts |
| Game Tools Gen I utils | https://github.com/RyudoSynbios/game-tools-collection/blob/88d5862bfa3cc581515a795c879dd8f09a0d2874/src/lib/templates/pokemon-red-blue-and-yellow/saveEditor/utils.ts |
| PCCS inspected tree | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard/tree/0186b4ebb05948bebda90ec6bc679b915b6bd893 |
| PCCS conversion source | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard/blob/0186b4ebb05948bebda90ec6bc679b915b6bd893/source/GBPokemon.cpp |
| PCCS box source | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard/blob/0186b4ebb05948bebda90ec6bc679b915b6bd893/source/PokeBox.cpp |
| Historical PCCS URL | https://github.com/GearsProgress/Pokemon-Community-Conversion-Standard |
| Poke Transporter GB inspected tree | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB/tree/34732ca884cd0b400932a6552f042e5cf6109437 |
| Poke Transporter flash source | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB/blob/34732ca884cd0b400932a6552f042e5cf6109437/source/flash_mem.cpp |
| Poke Transporter transfer source | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB/blob/34732ca884cd0b400932a6552f042e5cf6109437/source/pokemon_party.cpp |
| `pret/pokered` inspected tree | https://github.com/pret/pokered/tree/d70d99ffbd329473d96eaaf19fd97c86d2220b7f |
| `pret/pokered` SRAM layout | https://github.com/pret/pokered/blob/d70d99ffbd329473d96eaaf19fd97c86d2220b7f/ram/sram.asm |
| `pret/pokered` WRAM layout | https://github.com/pret/pokered/blob/d70d99ffbd329473d96eaaf19fd97c86d2220b7f/ram/wram.asm |
| `pret/pokered` save routines | https://github.com/pret/pokered/blob/d70d99ffbd329473d96eaaf19fd97c86d2220b7f/engine/menus/save.asm |
| `pret/pokered` event constants | https://github.com/pret/pokered/blob/d70d99ffbd329473d96eaaf19fd97c86d2220b7f/constants/event_constants.asm |
| `pret/pokefirered` inspected tree | https://github.com/pret/pokefirered/tree/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d |
| `pret/pokefirered` save source | https://github.com/pret/pokefirered/blob/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d/src/save.c |
| `pret/pokefirered` global save structs | https://github.com/pret/pokefirered/blob/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d/include/global.h |
| `pret/pokefirered` Pokemon structs | https://github.com/pret/pokefirered/blob/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d/include/pokemon.h |
| `pret/pokefirered` flags | https://github.com/pret/pokefirered/blob/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d/include/constants/flags.h |
| `pret/pokefirered` vars | https://github.com/pret/pokefirered/blob/70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d/include/constants/vars.h |

## 18. Commit Hashes and Branches Inspected

| Project | Branch | Commit | Commit date | Subject |
| --- | --- | --- | --- | --- |
| Current local project | `main` | `e9e531b99657704e128a723de9337b13420a4925` | 2026-04-19 | Complete Gen I Pokemon decode (party + boxes), move lookup, and JSON/text exports |
| `pokered-save-editor` | `master` | `b387f34ef20f311d5ef55ba181047357821b56d2` | 2019-03-31 | Merge branch 'master' of github.com:junebug12851/pokered-save-editor |
| Game Tools Collection | `master` | `88d5862bfa3cc581515a795c879dd8f09a0d2874` | 2026-06-18 | feat: add Super Mario RPG: Legend of the Seven Stars (SNES) [save] |
| PCCS | `latest-release` | `0186b4ebb05948bebda90ec6bc679b915b6bd893` | 2026-02-28 | Add table-generator script to .gitignore |
| Poke Transporter GB | `latest-release` | `34732ca884cd0b400932a6552f042e5cf6109437` | 2026-03-25 | Update issue templates |
| `pret/pokered` | `master` | `d70d99ffbd329473d96eaaf19fd97c86d2220b7f` | 2026-06-12 | Fix some issues with C tools, and build them with -std=c17 (#587) |
| `pret/pokefirered` | `master` | `70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d` | 2026-06-07 | Fix broken missing return (#759) |
