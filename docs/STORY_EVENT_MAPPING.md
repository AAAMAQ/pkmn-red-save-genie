# Story And Event Mapping Draft

Status: Red-side semantic evidence available; FireRed target mapping not complete

## Current Red Evidence

Save Genie currently exports:

- named event flags;
- trainer battle flags;
- gym/story consistency;
- static and legendary encounter flags;
- major story milestones;
- Hall of Fame entries;
- missable objects;
- hidden items;
- hidden coins;
- visited towns/Fly destinations;
- current scripts;
- runtime/world-state fields.

## Mapping Rules

- Do not treat every raw `EVENT_*` bit as permanent history.
- Temporary room-state flags must remain separate from historical completion.
- Use Hall of Fame records as evidence for historical League completion.
- Badge bits and gym leader story flags should be compared, not silently repaired.
- FireRed-exclusive story state must be initialized by an explicit policy.

## Current Categories

| Category | Red Source | Conversion Class | Target Status |
|---|---|---|---|
| Badges | Badge byte and mirror | direct transfer | FireRed badge flags needed |
| Gym leaders | Named event flags plus badge consistency | semantic translation | FireRed flags needed |
| Trainers | pret-backed trainer flags | semantic translation | FireRed trainer flags needed |
| Rival battles | named event/story flags where known | semantic translation | FireRed rival progression needed |
| Static encounters | static/legendary flags | semantic translation | FireRed encounter flags needed |
| Fossils | fossil item/mon fields and story flags | semantic translation | FireRed fossil restoration policy needed |
| Gift Pokémon | event flags/missables | semantic translation | FireRed gift state needed |
| League completion | Hall of Fame evidence and room flags | semantic translation | FireRed post-League/Sevii policy needed |

## Sevii Warning

FireRed includes Sevii Islands progression that has no direct Pokémon Red equivalent. A reliable converter must choose a conservative target policy before enabling post-League or Cinnabar-adjacent state.
