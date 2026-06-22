# FireRed Master JSON Convention

Status: planned, not implemented

The future `.fred.json` should mirror the `.red.json` philosophy:

- versioned schema;
- raw physical FireRed save image;
- byte-identical FireRed reconstruction;
- active save-slot metadata;
- save section metadata;
- section checksums;
- Pokémon encryption and substructure metadata;
- decoded trainer, party, PC, Pokédex, inventory, flags, variables, location, and progression;
- coverage map;
- diagnostics;
- conversion provenance.

## Required Before Conversion Claims

Before Save Genie can trust converted FireRed output, FireRed must support:

```text
FireRed .sav -> .fred.json -> byte-identical FireRed .sav
```

This must work on real saves, not only synthetic fixtures.

## FireRed Research Topics

- Save A / Save B active slot selection.
- Save index/counter handling.
- Section IDs and rotating section order.
- Section signatures.
- Section checksums.
- Security key.
- Trainer info.
- Party and PC Pokémon structures.
- Pokémon encryption and substructure permutation.
- Pokédex, inventory, flags, variables, map/location.

Primary future source: `pret/pokefirered`.
