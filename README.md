# 🧬 Pkmn Red Save Genie

A Pokémon Red (Generation I) save research, validation, export, and safe-editing tool built in modern C++.

Pkmn Red Save Genie preserves the full Gen I `.sav` byte image, decodes the major gameplay concepts into readable data, exports a canonical lossless `.red.json` master save, and provides a conservative Safe Editor for a small set of well-understood fields.

The project intentionally does **not** invent meanings for every runtime, scratch, unused, or unknown byte. Those bytes are preserved and classified so future research can continue without data loss.

---

## 📌 Project Overview

Pokémon Red (1996, Game Boy) stores game progress inside a 32KB SRAM save file divided into banks containing structured data such as:

- Trainer information  
- Party Pokémon  
- PC Box storage  
- Event flags  
- Hall of Fame records  
- Map and coordinate data  
- Monetary values (BCD encoded)  
- Checksum validation bytes  

This project reconstructs the meaningful source-side structure in C++ using a layered, object-oriented architecture while preserving the exact original file bytes for archival round trips.

The system does **not** distribute ROMs or copyrighted data.

---

## Release Status

The Pokémon Red / Gen I side is complete for its intended research, verification, and converter-source role.

Complete Red-side milestones:

- Read-only parser/exporter for major Gen I gameplay data.
- Conservative Safe Editor MVP for money, coins, trainer/rival names, badges, and existing item quantities.
- Current box cache, Daycare, world-state, event, trainer, story, hidden item/coin, visited-town, and Hall of Fame decoding.
- Canonical `.red.json` master-save export with byte-identical reconstruction.
- Full `0x0000-0x7FFF` byte preservation and coverage classification.
- Conversion-ready Red semantic model for the future Red-to-FireRed pipeline.

Corrective validation in July 2026 found and fixed four semantic-decoder defects exposed by emulator testing of an independently generated save: a valid dot glyph was decoded lossily, boxed HP/level fields used incomplete offsets, dirty current-box state was described too weakly, and Hall of Fame internal species IDs were filtered as if they were National Dex numbers. Save Genie now emits lossless text tokens, complete boxed-record fields, explicit current/permanent box state, and fixed-slot Hall of Fame records. Parser acceptance remains evidence, not a substitute for emulator validation.

Authoritative public docs:

- `docs/release/GEN1_SAVE_COVERAGE.md`
- `docs/release/RELEASE_CHECKLIST.md`
- `docs/PROJECT_PUBLIC_RESEARCH_RELEASE.md`
- `docs/RED_MASTER_JSON_COMPLETION_MILESTONE.md`
- `docs/RED_MASTER_JSON_CONVENTION.md`
- `docs/RED_MASTER_JSON_ROUND_TRIP.md`
- `docs/RED_SAVE_COVERAGE.md`
- `docs/CONVERSION_MODEL.md`

FireRed save writing, `.fred.json`, Gen III Pokémon serialization, and whole-save Red-to-FireRed conversion remain future project phases and are not part of the completed Red Save Genie milestone.

## Credits And Research References

Save Genie uses its own C++ implementation, but several Gen I coverage decisions were cross-checked against Junebug/Twilight's Apache-2.0 Pokémon Red/Blue save editor work:

- `junebug12851/pokered-save-editor`
- `junebug12851/pokered-save-editor-2`

These projects are credited as important research references for world-state ranges, Daycare offsets, text/font data, editor safety lessons, and test strategy. `pret/pokered` remains the authority for event-symbol names, trainer-completion flags, and game behavior.

---

## 🏗 Architecture

The project is designed with strict separation of responsibility.

```
main.cpp
│
├── FileManipulation
│     - Safe disk I/O
│     - Backup creation
│     - Edited file naming
│
├── SaveStructure
│     - SaveBuffer (bounded byte access)
│     - Gen1Layout (offset truth source)
│     - Gen1TextCodec (Gen I charset decoding)
│     - BcdCodec (money/coin decoding)
│     - Gen1Checksum (main + bank validation)
│     - SaveValidator
│
├── ReadOnlyData
│     - TrainerSummary
│     - BoxStats
│     - FlagSummary
│     - HallOfFame parsing
│     - Plain-English translation layer
│
├── RedTestExports
│     - SaveGenieSummary.txt
│     - PokemonBoxes.json
│     - PokemonSummary.json
│
├── RedMasterJson
│     - Canonical .red.json export/import
│     - SHA-256 and physical image validation
│     - Byte-identical reconstruction
│     - Coverage and conversion-readiness sections
│
└── WriteOnlyData
      - Safe mutation layer
      - Strict validation
      - Automatic checksum repair
```

### Design Philosophy

- No arbitrary offset writes  
- No unchecked memory access  
- No blind hex editing  
- Every edit goes through validation  
- Every mutation repairs checksums  
- No-edit `.red.json` reconstruction uses raw `physicalImage`, not decoded fields

---

## 🔍 Features

### ✅ Save File Reading

- Trainer Name (Gen I charset decoded)
- Rival Name
- Trainer ID (correct endianness handling)
- Money (3-byte BCD)
- Coins (2-byte BCD)
- Badge bitfield decoding
- Map ID and coordinates
- Playtime (hours/minutes/seconds)

### ✅ PC Box Analysis

- Party Pokémon decode with species, nicknames, Original Trainer data, EXP, level, moves, PP, DVs, Stat Experience, and party live stats
- Permanent PC Boxes 1-12
- Current box cache at `0x30C0`
- Daycare Pokémon where present
- Per-box Pokémon count
- Average level calculation
- Structured Pokémon entry parsing

### ✅ Event Flags

- Bitfield parsing
- Total flags set
- Indexed flag reporting
- `pret/pokered` event-label reporting
- Complete known true/false named event flag list
- Trainer-completion rows such as `Trainer #1, Viridian Forest: true`
- Defeated-trainer, major-story, static-battle, and gym/badge consistency categories

### ✅ World / Player State Diagnostics

- 97 named current-script progress values from `pret/pokered`/Junebug metadata
- 228 named missable object/NPC/object-state flags
- 54 named hidden item flags with map coordinates
- 12 named hidden coin flags with Game Corner coordinates
- 11 visited town/Fly-destination flags with town names
- Bank 1 runtime-field ledger for known map/header/warp/status/trade/script fields
- Player block coordinates, movement mode, direction bytes, Safari state, door/warp flags, and selected story/runtime bits
- Daycare in-use flag and deposited Pokémon decode

### ✅ Hall of Fame Parsing (Bank 0)

- Reads all Hall of Fame records (up to 50)
- Parses each recorded team (6 Pokémon max)
- Displays species and level
- Skips output if no Hall of Fame entries exist
- Defensive parsing (Bank 0 has no checksum)

### ✅ Data Integrity

- Main checksum validation
- Bank-level checksum validation
- Per-box checksum verification
- Automatic checksum repair on edits

### ✅ Canonical `.red.json`

- Preserves the full `0x8000` standard SRAM region
- Preserves trailing bytes beyond `0x8000`
- Stores canonical uppercase continuous hex in `physicalImage`
- Reconstructs `[RECONSTRUCTED] <save-name>.sav` byte-identically
- Includes SHA-256 hashes for source, standard SRAM, and trailing data
- Includes decoded semantic data and a conversion-ready Red source model
- Keeps unknown/runtime/scratch bytes preserved instead of pretending they are fully understood

---

## 🧠 Technical Concepts Implemented

- Binary file parsing
- Structured memory modeling
- Bank segmentation handling
- Endianness correction (LE / BE)
- Bitfield decoding
- BCD numeric decoding
- Gen I custom text decoding and safe encoding for supported editor fields
- Lossless raw-byte archival export
- SHA-256 validation through the Red master JSON system
- Safe buffer bounds enforcement
- Defensive parsing for unverified regions
- Object-oriented modular design

---

## 📂 Save File Structure Coverage

The system models all Gen I save banks:

### Bank 0
- Hall of Fame records
- Persistent early-game data

### Bank 1 (Main Data)
- Trainer profile
- Party Pokémon
- Inventory
- Event flags
- Map data
- Pokédex
- Current box cache
- Daycare
- World/story/runtime state diagnostics

### Bank 2 & 3
- PC Boxes
- Box checksums
- Bank-level checksums

Coverage distinction:

- Every byte from `0x0000` through `0x7FFF` is preserved and classified.
- Major transferable gameplay concepts are decoded.
- Some runtime, scratch, padding, unused, and unknown regions are intentionally preserved without fake English meanings.

---

## 🛡 Safety Guarantees

- Offers backup creation before writing user-facing outputs
- Never overwrites original save
- Generates `(EDITED) <filename>.sav`
- Accepts standard `0x8000` saves and preserves trailing bytes in larger emulator/cartridge-dump files
- Verifies checksum before and after edits
- Writes reconstructed saves with collision-safe names

---

## 🚀 How to Build

Requires:

- C++17 or later
- Xcode / Clang / GCC

Compile manually:

```bash
g++ -std=c++17 *.cpp -o SaveGenie
```

Or build via the included Xcode project.

---

## 📜 Legal Notice

This project:

- Does NOT distribute Pokémon ROMs
- Does NOT distribute copyrighted save data
- Does NOT include proprietary assets

It is an independent reverse-engineering and file format research project.

Users must supply their own legally obtained save files.

---

## License

Original project code and project-authored documentation are available under
the MIT License. Copyright (c) 2026 MAQ / BiG MAQ Studios. See `LICENSE`.

The license includes a non-binding stewardship note asking users to keep this
work oriented toward education, research, archival preservation, and
retro-development, and not merely repackage it as software for sale. This is a
personal request, not an additional restriction on the MIT permissions.

Third-party references and materials retain their own licenses and attribution.

---

## 🎯 Motivation

This project serves as:

- A technical exploration of early handheld game architecture
- A structured reverse-engineering exercise
- A demonstration of safe binary mutation design
- A preservation-focused tool for retro game enthusiasts

Rather than editing raw hex blindly, Pkmn Red Save Genie treats the save file as a structured system, ensuring clarity, safety, and correctness.

---

## 📌 Future Roadmap

Completed for the Red-side Save Genie milestone:

- Full source-byte preservation through `.red.json`
- Major Gen I gameplay decoding
- Safe Editor MVP
- Current box cache
- Event/trainer/world-state decoding
- Coverage classification
- Conversion-ready Red semantic model

Future phases:

- Broader real-save regression corpus and release packaging
- Optional UI/Web/WASM presentation layer
- `.fred.json` FireRed master-save format
- FireRed save reader/writer
- Gen III Pokémon encryption/serialization
- Red-to-FireRed mapping tables and conversion reports
- Emulator load/save-again validation for converted FireRed saves

---

## 🧩 Why This Matters

Gen I Pokémon save files are a snapshot of 1990s cartridge architecture.

Reconstructing their internal logic requires:

- Careful binary analysis
- Understanding legacy encoding systems
- Strict structural modeling
- Defensive programming

Pkmn Red Save Genie demonstrates how modern software engineering principles can be applied to legacy binary systems.

---
## ▶ How to Use

### 1️⃣ Obtain Your Own Save File

This program requires a legally obtained Pokémon Red `.sav` file.

You can extract your save file from:
- A physical cartridge using a save dumper


⚠️ This project does **NOT** provide ROMs or save files.

---

### 2️⃣ Place the Save File

Place your `.sav` file in the same directory as the compiled executable.

Example:

```
/Pkmn Red Save Genie
    ├── SaveGenie (executable)
    ├── Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav
```

---

### 3️⃣ Update the Input Filename (if necessary)

Inside `main.cpp`, update this line to match your save file name:

```cpp
// Replace with your own legally obtained save file
const std::string inputPath =
    "Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav";
```

If your file has a different name, change the string accordingly.

You may also provide a full file path if the save is located elsewhere.

---

### 4️⃣ Build the Program

Compile using C++17 or later.

Example (manual build):

```bash
g++ -std=c++17 *.cpp -o SaveGenie
```

Or build using the included Xcode project.

---

### 5️⃣ Run the Program

Execute:

```bash
./SaveGenie
```

The program will:

- Load the save file
- Offer to create a backup:
  ```
  (BACKUP) <yourfile>.sav
  ```
- Validate the save structure
- Display a full readable summary
- Verify checksum integrity
- Optionally write legacy/test exports
- Optionally write canonical `.red.json`
- Optionally reconstruct the `.red.json` into a byte-identical `[RECONSTRUCTED] <yourfile>.sav`

If editing mode is enabled, it will create:

```
(EDITED) <yourfile>.sav
```

Your original save file will never be overwritten.

---

## 🔒 Safety Notes

- The original save file is never modified.
- A backup is recommended before edits.
- Checksums are validated and repaired automatically.
- Files shorter than `0x8000` are rejected.
- Files larger than `0x8000` are decoded using the first `0x8000` bytes and preserve trailing bytes unchanged in `.red.json`.
