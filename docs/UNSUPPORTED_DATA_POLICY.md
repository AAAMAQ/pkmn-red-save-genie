# Unsupported Data Policy

Status: draft

## Policy Choices

Every non-equivalent field must choose exactly one handling policy:

- `preserve_exactly`: keep in `.red.json` only.
- `translate_directly`: map to an equivalent target concept.
- `translate_semantically`: map meaning through an explicit rule.
- `approximate_deterministically`: generate a reproducible target value with a warning.
- `initialize_to_safe_default`: target-only field gets a documented default.
- `omit_with_warning`: do not write to target, but report it.
- `reject_conversion`: fail conversion until user or policy resolves the issue.

## Current Unsupported Or Policy-Required Cases

- Glitch species and invalid species IDs.
- Glitch or unsupported moves.
- Unsupported Gen I text glyphs.
- Items with no FireRed equivalent.
- Key items that should become story state instead of inventory.
- Gen I runtime or temporary flags.
- Current location with no safe FireRed target.
- Ambiguous story combinations.
- Excess inventory if target pocket limits conflict.
- FireRed-exclusive state such as Sevii progress.

## Reporting Requirement

The future converter must emit machine-readable and human-readable reports listing every unsupported, defaulted, approximated, omitted, or rejected field.
