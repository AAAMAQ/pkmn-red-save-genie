# Conversion Validation

Status: draft validation plan

## Current Verified Foundation

- Red `.sav -> .red.json -> .sav` byte-identical reconstruction.
- Red trailing-byte preservation.
- Red decoded semantic model.
- Red-side `conversionModel` draft.

## Required Before Reliable Converter Claim

- FireRed `.sav -> .fred.json -> .sav` byte-identical reconstruction.
- FireRed section checksum validation.
- FireRed Pokémon encryption/serialization validation.
- Complete mapping or rejection policy for every transferable concept.
- Multiple legitimate Red and FireRed real-save regression tests.
- Converted save loads in FireRed.
- Converted save can be saved again in FireRed.
- Post-save output remains structurally valid.

## Report Requirements

Every conversion must report:

- directly transferred fields;
- semantically translated fields;
- approximated fields;
- defaulted fields;
- omitted fields;
- rejected fields;
- warnings;
- confidence levels;
- source and target hashes;
- validation results.

The report must state whether the output is lossless for the represented concept, semantically equivalent, approximated, incomplete, or rejected.
