# Save Genie Gen I Release Checklist

Date: 2026-06-20

## Required Before First Public Gen I Release

- Xcode target builds only production files.
- Read-only workflow loads a save, preserves backup behavior, validates checksums, and writes `SaveGenieSummary.txt`, `PokemonBoxes.json`, and `PokemonSummary.json`.
- Safe Editor MVP supports only money, coins, trainer name, rival name, badges, existing bag item quantities, and existing PC item quantities.
- Safe Editor writes only a separate edited save, repairs the main checksum, reloads the edited file, verifies requested values, and reports changed offsets.
- Current Box Cache appears separately from permanent PC Boxes 1-12.
- Event output includes raw counts, all known verified `pret/pokered` event labels, true/false trainer-progress rows, static battle rows, story/world categories, and gym/badge consistency checks.
- World/player/daycare summaries are read-only and credited to Junebug/Twilight reference research where their offsets informed coverage.
- Automated synthetic tests cover binary helpers, BCD, text, checksum repair, party DV/PP decode, safe editor setters, event categories, world/player state, and Daycare.
- README documents supported games, limitations, backup behavior, generated outputs, and no-ROM/no-save policy.

## Out Of Scope For This Release

- Pokémon editing.
- Raw offset editing.
- FireRed save reader/writer.
- PCCS integration.
- Gen I to Gen III Pokémon conversion.
- Whole-save Red to FireRed conversion.

## Manual Release Validation

- Run read-only path and answer `n` at the edit prompt.
- Run edit path and cancel before changes.
- Run edit path with multiple safe edits, then verify checksum and changed-offset report.
- Confirm `PokemonBoxes.json` has `boxes` and `currentBoxCache`.
- Confirm `PokemonSummary.json` includes machine-readable event/trainer/story flags without exposing any event editing.
- Confirm no saves, ROMs, PDFs, or generated exports are staged.
