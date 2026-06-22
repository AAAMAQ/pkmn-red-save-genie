# Pokémon Conversion Policy Draft

Status: deterministic draft policy, not final legality policy

## Implemented In `.red.json`

Every exported party, PC box, current-box-cache, and daycare Pokémon now includes a `conversion` object with:

- source/target game labels;
- species mapping by National Dex number;
- held-item policy;
- draft DV-to-IV output;
- draft Stat Experience-to-EV output;
- deterministic generated-field seed;
- generated-field policy for PID, nature, gender, ability, friendship, met data, ball, language, and ribbons;
- warnings for capped EVs or status-policy issues.

## Draft Policies

| Field | Policy | Status |
|---|---|---|
| Species | Map by National Dex number, not Gen I internal ID | draft accepted |
| Level | Copy source level | draft accepted |
| EXP | Preserve source EXP unless target growth compatibility requires recalculation | future validation |
| Moves | Map by semantic move identity/name | future target ID table needed |
| PP | Preserve current PP and PP Ups where target move supports it | future target validation |
| Held item | `NONE` safe default | draft accepted |
| DVs to IVs | `IV = DV * 2 + 1` | deterministic draft |
| Gen I Special DV | Map to both Special Attack IV and Special Defense IV | deterministic draft |
| Stat Experience to EVs | `floor(statExp / 256)`, split Special to SpA/SpD, scale total to 510 if needed | deterministic approximation |
| PID/personality | Derive later from per-Pokémon deterministic seed | future Gen III codec needed |
| Nature | Derive deterministically from personality seed | future Gen III codec needed |
| Gender | Derive from FireRed species gender ratio and personality | future target table needed |
| Ability | Derive from FireRed species ability rules | future target table needed |
| Friendship | FireRed species default or documented policy value | policy required |
| Met location | Conversion-policy location | policy required |
| Met level | Source level | draft accepted |
| Origin game | `pokemon_red_virtualized_source` policy marker | policy required |
| Poké Ball | default Poké Ball policy | policy required |
| Language | English default policy | draft accepted |
| Ribbons | none safe default | draft accepted |

## Not Yet Proven

The policy is deterministic but not yet a legality guarantee. It still needs FireRed Pokémon encryption/serialization, target species/move/item tables, and emulator validation.
