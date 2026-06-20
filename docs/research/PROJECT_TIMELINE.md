# Project Timeline

Project: `pkmn-red-save-genie`

Creator: MAQ, Big MAQ Studio

Last updated: 2026-06-19

This timeline uses only supported dates. Where the exact date is not known, the milestone is listed as a period or marked as supplied-context evidence.

## 2024 - Original Public Idea

Evidence: Reddit discussion titled `Is it possible to export the Pokemon Red save file to FireRed, allowing you to continue your save file from Red in FireRed?`

Source: https://www.reddit.com/r/GameboyAdvance/comments/1h6c5yc/is_it_possible_to_export_the_pok%C3%A9mon_red_save/

Milestone: MAQ publicly explored whether a Pokemon Red save could be translated into a FireRed save so the player could continue the original Kanto journey in FireRed.

Technical significance: The early idea already separated two concepts: semantic AI-assisted mapping and direct save-file manipulation. Later research refined this into deterministic model-based conversion.

## 2024 - Personal Preservation Origin

Evidence: Supplied project-origin section in the 2026-06-19 documentation prompt.

Milestone: The project origin is tied to preserving a personally significant Pokemon Red cartridge and save after battery-backed save functionality failed.

Technical significance: The hardware repair and save-preservation problem led to the larger Red-to-FireRed conversion question.

## 2025 - Active Implementation Period Begins

Evidence: Supplied project-origin section in the 2026-06-19 documentation prompt.

Milestone: Active implementation began in 2025, alongside university applications, formal study, other programming projects, game-development work, and learning C++/binary file manipulation.

Technical significance: The project matured from speculative idea into staged engineering.

Git note: The current local Git repository begins on 2026-02-23, so 2025 implementation is supported by supplied discussion context rather than current Git commits.

## May 2025 - Early Expert Communication Context

Evidence: `tmp/pdfs/Save Data Converter Project with Save Ginie git  .txt`

Milestone: Supplied conversation archive records MAQ speaking with The Gears of Progress and mentioning travel/Boston/Harvard summer school context.

Technical significance: This period shows the project was being discussed with experienced community developers before the current repository history.

Accuracy note: The archive supports Harvard summer school context, but not enough detail to claim a specific Harvard C++ course or formal C++ learning period.

## July 31, 2025 - Written Expert Questions Preferred

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Gears could not do a voice meeting and preferred written Discord questions for accurate answers.

Technical significance: Written expert discussions became part of the research record.

## August 20, 2025 - Break the Converter Into Smaller Pieces

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Gears advised starting with generalized reading/writing of Gen I and Gen III save data.

Technical significance: This directly influenced the staged roadmap:

```text
Gen I reader
-> Gen I exports
-> safe editor
-> Gen III reader/writer
-> conversion layer
```

## September 30 / October 1, 2025 - Small Steps Lead to Large Results

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Gears encouraged incremental development and noted conversion standardization progress.

Technical significance: The project adopted a small verified steps mindset.

## October 5-6, 2025 - Zayaldrie Location and Flag Advice

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Zayaldrie advised that exact tile coordinate preservation is unnecessary for a first location conversion, and that flags are harder than location.

Technical significance: Future location strategy became:

```text
Gen I map ID
-> known-good FireRed map/x/y/warp
```

## October 13, 2025 - Pokemon Community Conversion Standard Mentioned

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Gears announced the first version of the Pokemon Community Conversion Standard.

Technical significance: PCCS became a possible future reference for Pokemon-level conversion.

## January 7-8, 2026 - Two-Sided Decoder/Encoder Plan

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: The plan clarified around using decompiled GB/GBA games, analyzing `.sav` files, converting both Gen I and Gen III into readable data, and then matching them together.

Technical significance: The project became a model-based binary translation problem, not a one-off script.

## February 14, 2026 - Official Releases Do Not Replace Save Research

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Discussion with Gears noted that official FireRed releases/remakes may not expose or preserve original save data.

Technical significance: The project was framed as data preservation and transparent save research, not merely replay access.

## February 23, 2026 - Current Repository Initial Commit

Evidence: Git commit `cf50de7 Initial commit - Save Genie core`

Files:

| File group | Added |
| --- | --- |
| C++ source | FileManipulation, SaveStructure, ReadOnlyData, WriteOnlyData, main |
| Headers | Matching module headers |
| Xcode | Project and scheme files |
| Ignore rules | `.gitignore` |

Technical significance: The current modular C++ Save Genie architecture begins here.

## February 23, 2026 - README and Usage Documentation

Evidence: Git commits `af1d209` and `bdae805`

Milestone: README and usage details were added.

Technical significance: The project was presented as a structured safe save reader/editor rather than a raw hex patcher.

## February 24, 2026 - Save Structure and Map Names

Evidence: Git commit `8779127 Update save structure and read-only data + included names for each Map ID`

Milestone: Save structure/read-only data expanded and map lookup names were added.

Technical significance: Human-readable map/location output became possible.

## February 25, 2026 - Hall of Fame, Species, Map, and Pokedex

Evidence: Git commit `0ea6e66 Add Hall of Fame, species lookup, map lookup, Pokedex decoding`

Milestone: Major reader features were added.

Technical significance: The project moved from simple trainer summary toward broader save decoding.

## March 3, 2026 - Inventory Decoding

Evidence: Git commit `daf7a21 Add Bbag + PC item box decoding with Gen1 item lookup`

Milestone: Bag and PC item box decoding were added with Gen I item lookup.

Technical significance: Save Genie began covering inventory and item IDs in a user-readable form.

## March 3, 2026 - Text Corrections

Evidence: Git commit `cb0b5e0 Correct Small Text Issues`

Milestone: Small text issues were corrected.

Technical significance: Continued cleanup and output polish.

## March 5, 2026 - WriteOnlyData Layer

Evidence: Git commit `9d80ba8 Add Code for Write only data`

Milestone: Safe editing layer was added.

Technical significance: The project established a future mutation boundary separate from read-only parsing.

## March 26, 2026 - AI Reliability Guidance

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: Gears advised that AI can help with repetitive coding and debugging but should not be trusted as final authority for niche GB/GBA internals.

Technical significance: The project adopted a verification-first AI workflow.

## April 18, 2026 - Bug Fixes

Evidence: Git commit `7b5c0c4 Added Some Bug Fixes`

Milestone: Bug fixes were applied shortly before the major decode milestone.

Technical significance: Pre-stabilization before full Pokemon decode/export.

## April 19, 2026 - Full Gen I Pokemon Decode Foundation

Evidence: Git commit `e9e531b Complete Gen I Pokemon decode (party + boxes), move lookup, and JSON/text exports`; project brain

Milestone:

| Feature | Status |
| --- | --- |
| Party decode | Implemented |
| PC Boxes 1-12 decode/export | Implemented |
| Move lookup | Implemented |
| DVs | Implemented |
| Stat Exp | Implemented |
| JSON exports | Implemented |
| Text summary export | Implemented |

Technical significance: This is the latest major repository checkpoint and current completed milestone.

Validation note: Party values are strong regression anchors. PC box details still need deeper plausibility verification because the generated artifact shows some implausible boxed levels.

## April 19, 2026 - Next Research Direction: FireRed Symbols

Evidence: `pkmn_red_save_genie_inspiration_and_references.md`

Milestone: After box decode and JSON export, Gears recommended looking at the symbol list for pret's FRLG decompilation for future flag information.

Technical significance: Flag mapping became the next major research track after Gen I Pokemon data.

## June 19, 2026 - External Projects Deep Analysis

Evidence: `docs/external_projects_deep_analysis.md`

Milestone: External repositories and references were analyzed:

| Project | Use |
| --- | --- |
| `pokered-save-editor` | Gen I editor coverage and current-box concept. |
| Game Tools Collection | Browser-local workflow. |
| PCCS | Pokemon conversion concepts. |
| Poke Transporter GB | Pokemon transfer and Gen III sector write concepts. |
| `pret/pokered` | Gen I source map. |
| `pret/pokefirered` | FireRed source map. |

Technical significance: Confirmed that no analyzed project was found to implement the full whole-save Red-to-FireRed pipeline.

## June 19, 2026 - Complete Research Documentation Pass

Evidence: This documentation set

Milestone: Created:

| File | Purpose |
| --- | --- |
| `PKMN_RED_SAVE_GENIE_RESEARCH_DOCUMENT.md` | Main technical research monograph. |
| `RESEARCH_FINDINGS_LOG.md` | Findings and implementation implications. |
| `OPEN_RESEARCH_QUESTIONS.md` | Research backlog. |
| `PROJECT_TIMELINE.md` | Supported project chronology. |
| `REFERENCES.md` | Source and reference index. |

Technical significance: Establishes a durable documentation base for future contributors, research review, and converter development.

## Next Planned Milestones

| Order | Milestone | Evidence |
| --- | --- | --- |
| 1 | Verify PC box decode. | Project brain and current research finding R-008. |
| 2 | Add current box cache decode. | Project brain and external analysis. |
| 3 | Expand compact summary JSON. | Project brain. |
| 4 | Label Gen I event/trainer flags. | Gears advice and `pret/pokered` source map. |
| 5 | Implement safe WriteOnlyData MVP. | Current WriteOnlyData partial implementation. |
| 6 | Create save coverage report. | Project brain roadmap. |
| 7 | Begin FireRed reader. | Gen III research roadmap. |
| 8 | Begin FireRed writer. | Gen III research roadmap. |
| 9 | Integrate Pokemon conversion adapter. | PCCS/Poke Transporter research. |
| 10 | Build limited whole-save conversion proof. | Long-term converter roadmap. |
