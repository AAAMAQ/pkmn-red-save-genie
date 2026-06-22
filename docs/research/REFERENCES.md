# References

Project: `pkmn-red-save-genie`

Creator: MAQ, Big MAQ Studio

Last updated: 2026-06-22

## Primary Project Code Sources

| Source | Role |
| --- | --- |
| `README.md` | Public project overview, usage, safety notes, roadmap. |
| `.gitignore` | Legal/safety artifact exclusions: saves, ROMs, PDFs, generated exports. |
| `Pkmn Red Save Genie/CPP Files/FileManipulation.cpp` | Disk I/O, backup, write helpers. |
| `Pkmn Red Save Genie/HPP Files/FileManipulation.hpp` | File I/O API. |
| `Pkmn Red Save Genie/CPP Files/SaveStructure.cpp` | SaveBuffer, lookups, text codec, BCD, checksums, validation. |
| `Pkmn Red Save Genie/HPP Files/SaveStructure.hpp` | Gen I layout constants, lookup class declarations, codec/checksum APIs. |
| `Pkmn Red Save Genie/CPP Files/ReadOnlyData.cpp` | Main read-only decoding implementation. |
| `Pkmn Red Save Genie/HPP Files/ReadOnlyData.hpp` | Save models and read-only API. |
| `Pkmn Red Save Genie/CPP Files/WriteOnlyData.cpp` | Safe edit MVP implementation. |
| `Pkmn Red Save Genie/HPP Files/WriteOnlyData.hpp` | Safe edit request/result API. |
| `Pkmn Red Save Genie/CPP Files/main.cpp` | App flow, safe prompts, legacy exports, master `.red.json` export/reconstruction prompts. |
| `Pkmn Red Save Genie/CPP Files/RedMasterJson.cpp` | Canonical `.red.json` export/import/validation/reconstruction and coverage/conversion sections. |
| `Pkmn Red Save Genie/HPP Files/RedMasterJson.hpp` | Red master JSON API and result/options models. |
| `Pkmn Red Save Genie/CPP Files/RedTestExports.cpp` | Legacy/test export writers for summary, boxes, and compact JSON. |
| `Pkmn Red Save Genie/HPP Files/RedTestExports.hpp` | Legacy/test export API. |
| `Pkmn Red Save Genie.xcodeproj/project.pbxproj` | Xcode project structure. |
| `Pkmn Red Save Genie.xcodeproj/xcshareddata/xcschemes/Pkmn Red Save Genie.xcscheme` | Xcode scheme. |
| `tests/savegenie_core_tests.cpp` | Synthetic core parser/editor/helper tests. |
| `tests/red_master_json_tests.cpp` | `.red.json` round-trip, validation, semantic hierarchy, deterministic export, and collision tests. |

## Project Context and Planning Documents

| Source | Role |
| --- | --- |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/pkmn_red_save_genie_project_brain.md` | Current project brain, operating manual, tested offsets, roadmap, regression anchors. |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/pkmn_red_save_genie_inspiration_and_references.md` | Inspiration links, expert advice, strategic context, timeline of Gears/Zayaldrie advice. |
| `docs/external_projects_deep_analysis.md` | Detailed analysis of external projects and references. |
| `docs/PROJECT_GIT_LOG.md` | Public milestone and commit decision log. |
| `docs/PROJECT_PUBLIC_RESEARCH_RELEASE.md` | Final public research-release conclusion and authoritative documentation map. |
| Current user documentation prompt, 2026-06-19 | Project-origin section: MAQ, Big MAQ Studio, uncle's Pokemon Red cartridge, battery repair, personal preservation motivation. |

## Supplied PDF Text Extracts

The repository contains PDFs in `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/`. Extracted text copies were inspected under `tmp/pdfs/`.

| Extracted text source | Original PDF/topic | Use |
| --- | --- | --- |
| `tmp/pdfs/Save data structure (Generation I) - Bulbapedia, the community-driven Pokemon encyclopedia.txt` | Bulbapedia Gen I save structure | Banks, offsets, checksum, main data, party, current box, boxes, BCD, Pokedex, flags. |
| `tmp/pdfs/Save data structure (Generation III) - Bulbapedia, the community-driven Pokemon encyclopedia.txt` | Bulbapedia Gen III save structure | 128 KiB save, two save blocks, sections, signatures, checksums, save index, trainer/team/items data. |
| `tmp/pdfs/List of Pokemon by index number in Generation I - Bulbapedia, the community-driven Pokemon encyclopedia.txt` | Gen I species internal index | Species lookup and internal ID vs Pokedex distinction. |
| `tmp/pdfs/List of items by index number in Generation I - Bulbapedia, the community-driven Pokemon encyclopedia.txt` | Gen I item index | Item lookup and invalid/glitch item caveats. |
| `tmp/pdfs/List of maps by index number (Generation I) - Glitch City Wiki.txt` | Gen I map index | Map lookup table source. |
| `tmp/pdfs/Red to FireRed transfer.txt` | Early Red-to-FireRed proposal and discussion | Origin of AI mapping and direct binary manipulation ideas; Gears advice excerpt. |
| `tmp/pdfs/Pokemon Red to Fire Red detailed conversation.txt` | Long project planning discussion | Save Genie roadmap, text export correction, FireRed architecture planning, staged model-based conversion. |
| `tmp/pdfs/Save Data Converter Project with Save Ginie git  .txt` | Project planning and early Save Genie discussion | Early context, prototype ideas, README generation, project direction. |

## Generated Validation Artifacts

These files are generated runtime/reference artifacts and should not be committed as production outputs unless intentionally preserved in a documentation-only context.

| Source | Use |
| --- | --- |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/SaveGenieSummary.txt` | Current readable output example, checksum validity, party regression values, save size `0x802c`. |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/PokemonSummary.json` | Compact summary export shape and party/count data. |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/PokemonBoxes.json` | Full Pokemon export shape and PC box validation warning evidence. |

## Historical Prototype Files

| Source | Use |
| --- | --- |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/BETA Pokemon Gen 1 to Gen 3 save data transfer/Pokemon Gen 1 to Gen 3 save data transfer/main.cpp` | Early direct save-patching prototype showing hard-coded offsets, copy/edit/checksum experiments, and why the current modular architecture was needed. |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/BETA Pokemon Gen 1 to Gen 3 save data transfer/ROM folder/PokemonBox.txt or a Json.txt` | Early intended Pokemon box export sketch. |
| `Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/BETA Pokemon Gen 1 to Gen 3 save data transfer/Pokemon Gen 1 to Gen 3 save data transfer.xcodeproj/project.pbxproj` | Historical prototype Xcode structure. |

Note: ROM and save binaries in the reference folder were intentionally not used as distributable evidence. They should not be committed or redistributed.

## Git Commit References

| Commit | Date | Subject | Significance |
| --- | --- | --- | --- |
| `cf50de7` | 2026-02-23 | Initial commit - Save Genie core | Initial current repository architecture. |
| `af1d209` | 2026-02-23 | Add full detailed README | README foundation. |
| `bdae805` | 2026-02-23 | Add full detailed how to use in README + a comment in the main.cpp | Usage documentation. |
| `8779127` | 2026-02-24 | Update save structure and read-only data + included names for each Map ID | Map names and structure improvements. |
| `0ea6e66` | 2026-02-25 | Add Hall of Fame, species lookup, map lookup, Pokedex decoding | Major reader expansion. |
| `daf7a21` | 2026-03-03 | Add Bbag + PC item box decoding with Gen1 item lookup | Inventory decoding. |
| `cb0b5e0` | 2026-03-03 | Correct Small Text Issues | Output/text cleanup. |
| `9d80ba8` | 2026-03-05 | Add Code for Write only data | Safe editing layer. |
| `7b5c0c4` | 2026-04-18 | Added Some Bug Fixes | Pre-milestone fixes. |
| `e9e531b` | 2026-04-19 | Complete Gen I Pokemon decode (party + boxes), move lookup, and JSON/text exports | Current major milestone. |
| `8d3e911` | 2026-06-20 | Advance Gen I Save Genie release foundation | Safe Editor MVP, current box cache, event/trainer flags, tests, release docs, and README updates. |
| `ade3701` | 2026-06-22 | Expand Gen I world-state decoding | Current scripts, missables, hidden items/coins, visited towns, Daycare/world-state refinements, and coverage updates. |
| `929b6a0` | 2026-06-22 | Complete Red master JSON conversion-ready milestone | Canonical `.red.json`, RedTestExports, schema, round-trip tests, conversion model, and Red-side completion docs. |

## External Projects and Repositories

| Project | URL | Inspected commit/reference | Use |
| --- | --- | --- | --- |
| `junebug12851/pokered-save-editor` | https://github.com/junebug12851/pokered-save-editor | `b387f34ef20f311d5ef55ba181047357821b56d2` | Gen I editor coverage, current box cache, checksums, Angular/Electron UI ideas. |
| Game Tools Collection | https://github.com/RyudoSynbios/game-tools-collection | `88d5862bfa3cc581515a795c879dd8f09a0d2874` | Browser-local save workflow, declarative schema, Red/Blue/Yellow save editing. |
| Game Tools live editor | https://game-tools-collection.com/pokemon-red-blue-and-yellow/save-editor | Live URL opened; source repo used for code evidence | Browser-local UX reference. |
| Pokemon Community Conversion Standard | https://github.com/Striaton-Lab-Team/Pokemon-Community-Conversion-Standard | `0186b4ebb05948bebda90ec6bc679b915b6bd893` | Pokemon-level Gen I/II to Gen III conversion policy and code reference. |
| Historical PCCS URL | https://github.com/GearsProgress/Pokemon-Community-Conversion-Standard | Historical/moved link | Referenced by Poke Transporter `.gitmodules`. |
| Poke Transporter GB | https://github.com/Striaton-Lab-Team/Poke_Transporter_GB | `34732ca884cd0b400932a6552f042e5cf6109437` | Pokemon transfer, PCCS integration, Gen III save-sector write concepts. |
| `pret/pokered` | https://github.com/pret/pokered | `d70d99ffbd329473d96eaaf19fd97c86d2220b7f` | Gen I SRAM, WRAM, save routines, event constants, maps, text, species/items/moves. |
| `pret/pokefirered` | https://github.com/pret/pokefirered | `70b76a15df8c6a6dd03d7a09e8f9eddd2e4dc29d` | FireRed save sections, structs, flags, vars, maps, Pokemon data. |
| Current public repo | https://github.com/AAAMAQ/pkmn-red-save-genie | Local `929b6a0` at this documentation pass; `origin/main` may lag behind local commits until pushed | Public project identity; local repo remains source of truth. |

## External Documentation and Web Sources

| Source | URL | Use |
| --- | --- | --- |
| Reddit original proposal | https://www.reddit.com/r/GameboyAdvance/comments/1h6c5yc/is_it_possible_to_export_the_pok%C3%A9mon_red_save/ | Public early proposal, AI mapping idea, direct save manipulation idea, community feedback. |
| Bulbapedia Gen I save structure | https://bulbapedia.bulbagarden.net/wiki/Save_data_structure_(Generation_I) | Gen I save layout reference. |
| Bulbapedia Gen III save structure | https://bulbapedia.bulbagarden.net/wiki/Save_data_structure_(Generation_III) | Gen III save layout reference. |
| Bulbapedia Gen I species index | https://bulbapedia.bulbagarden.net/wiki/List_of_Pokemon_by_index_number_in_Generation_I | Internal species ID reference. |
| Bulbapedia Gen I item index | https://bulbapedia.bulbagarden.net/wiki/List_of_items_by_index_number_in_Generation_I | Item ID reference. |
| Glitch City Wiki Gen I map index | https://glitchcity.wiki/wiki/List_of_maps_by_index_number_(Generation_I) | Map ID reference. |

## Expert Communications

| Person/source | Evidence file | Technical contribution |
| --- | --- | --- |
| The Gears of Progress | `pkmn_red_save_genie_inspiration_and_references.md`; `tmp/pdfs/Red to FireRed transfer.txt`; `tmp/pdfs/Save Data Converter Project with Save Ginie git  .txt` | Break project into pieces; separate Pokemon/flags/location; use decompilations; avoid AI guessing; document everything; consider PCCS. |
| Zayaldrie | `pkmn_red_save_genie_inspiration_and_references.md` | Safe location mapping, save diffs, Gen III section awareness, flags harder than coordinates, Sevii consistency warning. |
| Reddit community commenters | Reddit original proposal URL | Deterministic mapping feedback, Pokemon-level transfer references, manual flag-mapping warnings. |

## License and Reuse Notes From External Analysis

| Project | License finding | Reuse status |
| --- | --- | --- |
| `pokered-save-editor` | Apache-2.0 | Possible with attribution and compliance; best used conceptually. |
| Game Tools Collection | MIT | Possible with attribution; best used for browser workflow/schema ideas. |
| Poke Transporter GB | MIT | Possible with attribution; hardware-specific code should remain conceptual. |
| PCCS | No clear root license found in inspected clone | Legal review required before direct reuse/vendor. |
| `pret/pokered` | No root license found in inspected clone | Treat as reference-only until clarified. |
| `pret/pokefirered` | No root license found in inspected clone | Treat as reference-only until clarified. |

## Citation Guidance for Future Docs

Use stable source paths for implementation claims, for example:

```text
Pkmn Red Save Genie/CPP Files/ReadOnlyData.cpp
ReadOnlyData::GetPartyAsBox0()
```

Use commit hashes for historical claims, for example:

```text
e9e531b Complete Gen I Pokemon decode (party + boxes), move lookup, and JSON/text exports
```

Use supplied discussion filenames for project-origin and expert-advice claims, for example:

```text
Pkmn Red Save Genie/IMPORTANT PDF and other files IGNORE/pkmn_red_save_genie_inspiration_and_references.md
```

Use external links for public/reference claims, but do not copy large source or article text into project documentation.
