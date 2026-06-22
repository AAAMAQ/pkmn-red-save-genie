# Save Genie Gen I Release Checklist

Date: 2026-06-20

Status update: complete for the intended Pokémon Red Save Genie research, verification, archival, safe-edit MVP, and converter-source role. Remaining items are public-release polish, optional evidence expansion, or future FireRed/converter work.

## Completed For The Red-Side Release

- Xcode target builds only production files.
- Read-only workflow loads a save, preserves backup behavior, validates checksums, and writes `SaveGenieSummary.txt`, `PokemonBoxes.json`, and `PokemonSummary.json`.
- Safe Editor MVP supports only money, coins, trainer name, rival name, badges, existing bag item quantities, and existing PC item quantities.
- Safe Editor writes only a separate edited save, repairs the main checksum, reloads the edited file, verifies requested values, and reports changed offsets.
- Current Box Cache appears separately from permanent PC Boxes 1-12.
- Event output includes raw counts, all known verified `pret/pokered` event labels, true/false trainer-progress rows, static battle rows, story/world categories, and gym/badge consistency checks.
- World/player/daycare summaries are read-only and credited to Junebug/Twilight reference research where their offsets informed coverage.
- Automated synthetic tests cover binary helpers, BCD, text, checksum repair, party DV/PP decode, safe editor setters, event categories, world/player state, and Daycare.
- README documents supported games, limitations, backup behavior, generated outputs, and no-ROM/no-save policy.
- Canonical `.red.json` preserves source bytes, trailing data, hashes, coverage, decoded semantics, and conversion-readiness metadata.
- `.red.json` reconstructs byte-identically in the automated synthetic round-trip harness.

## Accepted Human Verification

For the public research-release documentation pass, MAQ's reported human verification, playtesting, emulator checks, and bug-testing results are accepted as correct unless contradicted by repository evidence.

This accepted evidence supports the conclusion that the Red-side Save Genie can be documented as complete for its intended source role. Future validation can expand the public evidence base, but should not reopen the completed Red master JSON design without a concrete bug.

## Out Of Scope For This Release

- Pokémon editing.
- Raw offset editing.
- FireRed save reader/writer.
- PCCS integration.
- Gen I to Gen III Pokémon conversion.
- Whole-save Red to FireRed conversion.

## Manual Release Validation

These remain useful as repeatable release checks, not blockers to the Red-side source-completion conclusion:

- Run read-only path and answer `n` at the edit prompt.
- Run edit path and cancel before changes.
- Run edit path with multiple safe edits, then verify checksum and changed-offset report.
- Confirm `PokemonBoxes.json` has `boxes` and `currentBoxCache`.
- Confirm `PokemonSummary.json` includes machine-readable event/trainer/story flags without exposing any event editing.
- Confirm no saves, ROMs, PDFs, or generated exports are staged.
