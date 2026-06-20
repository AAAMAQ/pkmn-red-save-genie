# 🧬 Pkmn Red Save Genie

A full-structure Pokémon Red (Generation I) save file reader and safe editor built in modern C++.

Pkmn Red Save Genie reverse-engineers the complete Gen I `.sav` format across all banks and transforms raw binary data into structured, human-readable English while preserving file integrity and checksum correctness.

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

This project reconstructs that internal structure in C++ using a layered, object-oriented architecture.

The system does **not** distribute ROMs or copyrighted data.

---

## Release Status

The current Gen I Save Genie release path is tracked in:

- `docs/release/GEN1_SAVE_COVERAGE.md`
- `docs/release/RELEASE_CHECKLIST.md`

The Gen I reader/exporter is the current production focus. FireRed save writing, PCCS integration, and whole-save Red-to-FireRed conversion remain future project phases and are not required for the first Save Genie release.

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

- Current script byte-region nonzero count
- Missable object/NPC/object-state flag counts
- Hidden item and hidden coin flag counts
- Visited town/Fly-destination flag counts
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

---

## 🧠 Technical Concepts Implemented

- Binary file parsing
- Structured memory modeling
- Bank segmentation handling
- Endianness correction (LE / BE)
- Bitfield decoding
- BCD numeric decoding
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

### Bank 2 & 3
- PC Boxes
- Box checksums
- Bank-level checksums

---

## 🛡 Safety Guarantees

- Always creates `(BACKUP) <filename>.sav`
- Never overwrites original save
- Generates `(EDITED) <filename>.sav`
- Performs strict size validation (0x8000 bytes expected)
- Verifies checksum before and after edits

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

## 🎯 Motivation

This project serves as:

- A technical exploration of early handheld game architecture
- A structured reverse-engineering exercise
- A demonstration of safe binary mutation design
- A preservation-focused tool for retro game enthusiasts

Rather than editing raw hex blindly, Pkmn Red Save Genie treats the save file as a structured system, ensuring clarity, safety, and correctness.

---

## 📌 Future Roadmap

- Full party Pokémon stat decoding
- Complete PC Box Pokémon decoding
- Species name lookup integration
- Move set decoding
- Item inventory parsing
- CLI command support
- WebAssembly build for browser-based editing

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
- Create a backup:
  ```
  (BACKUP) <yourfile>.sav
  ```
- Validate the save structure
- Display a full readable summary
- Verify checksum integrity

If editing mode is enabled, it will create:

```
(EDITED) <yourfile>.sav
```

Your original save file will never be overwritten.

---

## 🔒 Safety Notes

- The original save file is never modified.
- A backup is always created before edits.
- Checksums are validated and repaired automatically.
- Only properly structured 32KB Gen I save files are supported.
