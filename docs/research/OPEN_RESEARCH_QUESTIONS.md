# Open Research Questions

Project: `pkmn-red-save-genie`

Creator: MAQ, Big MAQ Studio

Last updated: 2026-06-22

Status note: questions in this file are retained as research history. Several Gen I questions are now answered for the completed Red-side Save Genie and `.red.json` source role, while broader corpus validation or FireRed target mapping may remain future work.

## Q-GEN1-001 - How should unsupported Gen I text bytes be normalized?

Question: How should unsupported Gen I text bytes, lowercase-like glyphs, punctuation, or modded text be represented in exports and future edits?

Why it matters: Current output uses `?` for unknown glyphs. This is safe but lossy.

Current evidence: `Gen1TextCodec` handles A-Z, 0-9, space, and terminator. PEGGY/MARIO decodes correctly. Names like `BRU?O` and `78??` show unsupported bytes.

Likely source: `pret/pokered/constants/charmap.asm`; existing editor text tables; controlled byte fixtures.

Experiment needed: Generate byte fixtures for the full known charmap and verify decode/encode behavior.

Priority: Medium

Status: Mostly answered for current Save Genie support; broader glyph coverage remains optional/future polish.

## Q-GEN1-002 - How should regional save differences be detected?

Question: How should the tool distinguish English Red/Blue/Yellow saves from Japanese or other regional variants?

Why it matters: Offsets and structures may differ by version or region.

Current evidence: Current constants target North American/English-style Gen I saves. Game Tools Collection has separate Japan utilities.

Likely source: Bulbapedia Gen I save structure, Game Tools Collection Japan utilities, pret version branches if available.

Experiment needed: Build layout profiles and detect version/region by safe signatures or user selection.

Priority: Medium

Status: Open for multi-region support. The completed Red-side milestone assumes English/USA-Europe style Pokémon Red/Blue-compatible saves unless later verified.

## Q-GEN1-003 - Should current box cache or permanent box copy be authoritative?

Question: When the current box cache differs from the permanent PC box copy, which should Save Genie export as authoritative?

Why it matters: The current in-game box cache at `0x30C0` can be fresher than the bank copy.

Current evidence: `Gen1Layout` has current box cache constants. External analysis confirms other editors overlay the current box cache.

Likely source: `pret/pokered` save/change-box routines; save-diff experiments after changing boxes.

Experiment needed: Compare a save before and after depositing/withdrawing Pokemon and switching PC boxes.

Priority: Critical

Status: Answered for export architecture: Save Genie exports current box cache separately and records synchronization metadata. Broader real-save edge-case validation remains useful.

## Q-GEN1-004 - Is the boxed Pokemon level offset correct in current code?

Question: Why do some generated PC box levels appear above 100, and should boxed level be read from `+0x03`, `+0x20`, `+0x21`, or another source?

Why it matters: Box export is a major milestone but implausible levels can corrupt downstream conversion.

Current evidence: `PokemonBoxes.json` contains boxed levels above 100. `DecodeBoxMonStruct()` reads `monOff + 0x21` even though `BoxStructSize` is `0x21`; the nearby comment says many tools treat level as `+0x03`.

Likely source: Bulbapedia Gen I Pokemon structures, `pret/pokered`, existing save editors, controlled known-box screenshots.

Experiment needed: Create a controlled box with known level Pokemon, save, dump bytes, and compare output.

Priority: Critical

Status: Accepted as verified for the completed Red-side source role based on current implementation and reported human validation. Additional real-save corpus testing remains useful evidence expansion.

## Q-GEN1-005 - Which Gen I save ranges are decoded, known, unused, or unknown?

Question: What is the complete coverage map for `0x0000-0x7FFF`?

Why it matters: The project wants to become a reliable Save Genie foundation, not just a partial summary tool.

Current evidence: `docs/release/GEN1_SAVE_COVERAGE.md` now classifies `0x0000-0x7FFF`, and `Gen1Layout`/`WorldStateSummary` include named current scripts, missables, hidden items, hidden coins, visited towns/Fly destinations, Daycare, current box cache, event labels, and many Bank 1 runtime fields. Some runtime/cache/union ranges remain intentionally not assigned English meanings.

Likely source: `SaveStructure.hpp`, Bulbapedia Gen I, `pret/pokered/ram/sram.asm`, `pret/pokered/ram/wram.asm`.

Experiment needed: Validate the current coverage map against real save-diff experiments, then decide whether current-map warp/sign/sprite cache rows should become structured exports.

Priority: Medium

Status: Answered for preservation and converter-source coverage: every standard SRAM byte is preserved/classified. Deeper runtime/cache English interpretation remains optional research.

## Q-FLAG-001 - Which Gen I event flags correspond to defeated trainers?

Question: Which event flag indices map to `EVENT_BEAT_*` symbols in Pokemon Red/Blue?

Why it matters: Defeated trainers are essential to story/progress conversion.

Current evidence: `ReadOnlyData::GetEventFlagSummary()` decodes all known `pret/pokered` event labels at `0x29F3`, and `ReadOnlyData::GetEventCategorySummary()` now derives true/false trainer-progress rows from `EVENT_BEAT_*_TRAINER_N` symbols. `pret/pokered/scripts/ViridianForest.asm` confirms trainer macro usage for `EVENT_BEAT_VIRIDIAN_FOREST_TRAINER_0`.

Likely source: `pret/pokered/constants/event_constants.asm`.

Experiment needed: Validate a broad trainer set against real save diffs and add friendlier labels for non-numbered story battles where needed.

Priority: Critical

Status: Answered for Gen I read-only decoding and Red-side `.red.json` source modeling. FireRed conversion mapping remains open under Q-FLAG-002.

## Q-FLAG-002 - Which FireRed flags correspond to Gen I trainer and story events?

Question: Which FireRed flags/vars match, approximate, or have no equivalent for Gen I trainer and story events?

Why it matters: Whole-save conversion cannot preserve story state without verified target flags.

Current evidence: External analysis identified `pret/pokefirered/include/constants/flags.h`, `vars.h`, and map scripts as sources.

Likely source: `pret/pokefirered/include/constants/flags.h`, `include/constants/vars.h`, `data/maps/**/scripts.inc`.

Experiment needed: Controlled FireRed save diffs after defeating trainers, collecting gifts, and passing major milestones.

Priority: Critical

Status: Open

## Q-FLAG-003 - Which Gen I story milestones have no exact FireRed equivalent?

Question: Which Red/Blue events cannot be cleanly represented in FireRed?

Why it matters: Conversion reports must state what was approximated or skipped.

Current evidence: Expert advice says flags are tedious and not one-to-one. FireRed has additional content and altered scripts.

Likely source: `pret/pokered`, `pret/pokefirered`, save diffs, map scripts.

Experiment needed: Create a milestone mapping table with statuses: direct, approximate, unsupported, unsafe.

Priority: High

Status: Open

## Q-GEN3-001 - What FireRed state is necessary for a stable post-League save?

Question: Which FireRed flags, vars, and Pokedex fields must be synchronized for a stable post-League or postgame-compatible save?

Why it matters: A bad postgame state can produce impossible or broken progression.

Current evidence: External analysis points to National Dex flags/vars, Elite Four/Hall of Fame scripts, and Sevii scripts.

Likely source: `pret/pokefirered/src/event_data.c`, `include/constants/flags.h`, `include/constants/vars.h`, Hall of Fame scripts, Sevii scripts.

Experiment needed: Compare pre-League, post-League, and post-Sevii saves.

Priority: Critical

Status: Open

## Q-GEN3-002 - How should Sevii Islands progress be initialized?

Question: Should initial conversion leave Sevii content untouched, initialize a safe pre-Sevii state, or set a postgame-compatible Sevii state?

Why it matters: Sevii Islands are FireRed-exclusive and can conflict with converted Kanto progress.

Current evidence: Zayaldrie warned that Sevii 1-3 occur before Cinnabar Gym in FRLG progression.

Likely source: `pret/pokefirered` Sevii scripts, seagallop scripts, `field_specials.c`, save diffs.

Experiment needed: Build a small set of safe template states and test emulator loads.

Priority: Critical

Status: Open

## Q-GEN3-003 - What is the safest first FireRed write operation?

Question: After a read-only FireRed section parser exists, what should be edited first?

Why it matters: FireRed writes are risky because of section checksums, save slots, and security key behavior.

Current evidence: External analysis recommends unchanged round-trip first, then a low-risk SaveBlock2 field such as player name.

Likely source: `pret/pokefirered/src/save.c`, `include/global.h`, Bulbapedia Gen III save structure.

Experiment needed: Read a copied save, rewrite unchanged, then edit player name and validate emulator load.

Priority: Critical

Status: Open

## Q-LOC-001 - What location variables beyond map/x/y must be written?

Question: Which FireRed fields besides map group/map ID/x/y/warp must be synchronized for a safe load?

Why it matters: Bad location state can softlock or load inconsistent maps.

Current evidence: `SaveBlock1` includes multiple location and warp fields. Zayaldrie advised safe map/x/y first.

Likely source: `pret/pokefirered/include/global.h`, map scripts, warp data, save diffs.

Experiment needed: Move between maps, save, diff location-related fields.

Priority: High

Status: Open

## Q-CONV-001 - How should Gen I Stat Exp become legal Gen III EVs?

Question: What deterministic rule should convert Gen I Stat Exp to Gen III EVs while respecting the 510 total EV cap?

Why it matters: Raw Stat Exp cannot be copied directly into Gen III.

Current evidence: Current `PokemonMon` exposes all five Stat Exp values. PCCS inspected path currently zeros EVs.

Likely source: PCCS, Pokemon transfer standards, legality research.

Experiment needed: Compare policies: zero EVs, scaled EVs, capped proportional EVs, or mode-specific choices.

Priority: High

Status: Open

## Q-CONV-002 - How should Gen I DVs become Gen III IVs?

Question: Should IVs be `DV * 2`, `DV * 2 + 1`, PCCS-generated, or policy-dependent?

Why it matters: IV choice affects stats, legality, shininess, gender, Hidden Power, and player trust.

Current evidence: Current Save Genie exports DVs. PCCS inspected code does not simply use `DV * 2`.

Likely source: PCCS, Poke Transporter GB, community conversion standards.

Experiment needed: Define and test deterministic policy outputs.

Priority: High

Status: Open

## Q-CONV-003 - Which conversion policy should be default?

Question: Should the default prioritize faithfulness, legality, virtual-console style behavior, or original-project policy?

Why it matters: Different users may expect different outcomes.

Current evidence: PCCS discusses Faithful, Legal, Virtual, and Original modes, but inspected code currently implements Original path.

Likely source: PCCS docs/code, project goals, user-facing converter design.

Experiment needed: Draft policy docs and compare outputs for sample Pokemon.

Priority: High

Status: Open

## Q-CONV-004 - How should glitch Pokemon be handled?

Question: Should invalid species be rejected, sanitized, preserved in report only, or converted to safe placeholder species?

Why it matters: Gen I saves can contain glitch species and invalid data.

Current evidence: Species/item references include invalid/glitch ranges. PCCS maps MissingNo to Porygon in inspected path.

Likely source: PCCS, Bulbapedia glitch species references, project policy.

Experiment needed: Create synthetic invalid species fixtures and define user-facing behavior.

Priority: Medium

Status: Open

## Q-REPORT-001 - How should conversion decisions be reported to users?

Question: What should a conversion report show for preserved, generated, changed, skipped, unsafe, and unsupported data?

Why it matters: Whole-save conversion will require compromises. Users need transparency.

Current evidence: Current project already exports text and JSON summaries. Future architecture recommends `ConversionReport` and `ValidationReport`.

Likely source: Current JSON/text export design, external tool UX, preservation ethics.

Experiment needed: Design sample conversion report schema before implementing converter.

Priority: High

Status: Open
