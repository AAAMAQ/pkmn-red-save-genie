# Red To FireRed Mapping Draft

Status: draft mapping table
Primary source in code: `conversionModel.conversionPolicies`

## Mapping Principles

- Do not assume numeric IDs match across generations.
- Prefer semantic identity over raw byte identity.
- Preserve source values and source paths in `.red.json`.
- Report every dropped, approximated, defaulted, or unsupported concept.

## Current Mapping Table

| Concept | Red Source | FireRed Target Basis | Status | Notes |
|---|---|---|---|---|
| Species | Gen I internal species ID plus National Dex number | National Dex number | direct/translated | Gen I internal species IDs are not FireRed species IDs. |
| Moves | Gen I move ID plus move name | Semantic move identity/name | policy draft | FireRed numeric move IDs must be resolved later. |
| Bag items | Gen I item ID plus item name | Semantic item identity/name | mixed | Direct items map by name; TMs/HMs map by taught move; key items may become story state. |
| PC items | Gen I item ID plus item name | Semantic item identity/name | mixed | Same policy as bag items. |
| TMs/HMs | Gen I item entry | Taught move identity | semantic translation | Do not copy item IDs directly. |
| Trainer name | Gen I text | FireRed text | direct with encoding policy | Unsupported characters require a warning or user policy. |
| Rival name | Gen I text | FireRed rival/state equivalent | policy required | FireRed rival naming/state location needs target research. |
| Trainer ID | Big-endian Gen I value | FireRed trainer ID policy | direct/policy | Exact TID/SID split requires FireRed policy. |
| Badges | Badge bitfield | FireRed badge flags | direct plus story policy | Badge bits transfer, related story flags are separate. |
| Pokédex | Owned/seen bitsets | FireRed Pokédex seen/owned | direct by National Dex | National Dex availability policy remains target-side. |
| Location | Map ID and coordinates | Safe FireRed map/group/x/y/warp | semantic translation | Use known-good spawn points first. |
| Story flags | Named events and evidence | FireRed flags/vars | semantic translation | Temporary flags must not be treated as history. |
| Hall of Fame | Bank 0 records | League-completion evidence | semantic translation | Direct FireRed Hall of Fame writing is future research. |
| Current box | Current box byte/cache | FireRed box selection/storage state | semantic translation | Cache/permanent sync should be validated on real saves. |

## Current Code Support

`.red.json` now includes per-Pokémon conversion records and a top-level `conversionModel` summary. FireRed target IDs and save offsets are intentionally not written yet.
