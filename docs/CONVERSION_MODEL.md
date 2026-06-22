# Shared Conversion Model

Status: complete Red-side source model; FireRed target model planned
Current implementation: `.red.json` contains a top-level `conversionModel` section.

## Purpose

The converter must not write directly from Pokémon Red offsets into FireRed offsets. The intended pipeline is:

```text
.red.json
  -> shared conversion model
  -> .fred.json
  -> FireRed save writer
```

The shared model stores transferable gameplay concepts without depending on either game's physical save layout.

## Implemented Red-Side Sections

The current `.red.json` `conversionModel` includes:

- `identity`: trainer name, trainer ID, source path, gender policy, target identity mapping.
- `trainer`: text policy and trainer metadata.
- `playtime`: hours, minutes, seconds.
- `currency`: money and coins.
- `badges`: badge byte and story-flag policy.
- `pokedex`: owned/seen counts and National Dex mapping policy.
- `party`: party count and per-Pokémon conversion records.
- `storage`: PC storage count and capacity policy.
- `daycare`: daycare transfer policy.
- `inventory`: item counts by conversion status.
- `storyProgress`: major milestones, gym consistency, and League evidence.
- `staticEncounters`: static/legendary battle state summary.
- `hallOfFame`: historical League-completion evidence policy.
- `locationPolicy`: map/location conversion policy.
- `conversionPolicies`: species, moves, items, text, status, DV/IV, StatExp/EV, and generated-field policies.
- `classificationTable`: machine-readable grouping of direct, semantic, Red-only, and unsupported concepts.
- `warnings`: important limitations.
- `provenance`: sources and research dependencies.

## Classification Values

- `direct_transfer`: transferable concept with a clear target semantic equivalent.
- `semantic_translation`: transferable only after mapping meaning, not raw bytes.
- `red_only_preservation`: preserved in `.red.json` but not intended for FireRed.
- `unsupported_or_policy_required`: requires rejection, user choice, or a target-side policy.
- `approximate_deterministically`: target value can be generated consistently but is not lossless.
- `initialize_to_safe_default`: FireRed-only field gets a documented default.

## Current Boundary

This document describes the completed Red source model only. It does not claim a FireRed writer or reliable converted save exists yet.

The next converter milestone is not more Red model redesign. It is proving the FireRed side with `.fred.json`, FireRed save-slot/section parsing, checksums, Gen III Pokémon serialization, and emulator validation.
