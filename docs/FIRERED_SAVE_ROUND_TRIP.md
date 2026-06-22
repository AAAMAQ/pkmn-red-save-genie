# FireRed Save Round Trip Plan

Status: planned, not implemented

## Goal

The FireRed side must eventually prove:

```text
original_firered.sav -> original.fred.json -> [RECONSTRUCTED] original_firered.sav
```

Expected result:

- identical file size;
- identical SHA-256;
- zero byte differences;
- valid active save slot;
- valid section checksums.

## Stop Rule

Do not generate or claim reliable Red-to-FireRed converted saves until FireRed round-trip reconstruction is proven.

## Minimum Tests

- Standard FireRed save round trip.
- Active-slot selection.
- Section order preservation.
- Section checksum validation.
- Pokémon encryption/decryption round trip.
- Party and PC storage round trip.
- Corrupt checksum rejection.
- Unsupported schema version rejection.
- Emulator load and save-again validation.
