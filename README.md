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
