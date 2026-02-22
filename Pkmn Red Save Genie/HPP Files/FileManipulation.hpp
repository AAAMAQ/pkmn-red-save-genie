//
//  FileManipulation.hpp
//  Pkmn Red Save Genie
//
//  Created by MAQ on 13/02/26.
//

#ifndef FileManipulation_hpp
#define FileManipulation_hpp

//
//  FileManipulation.hpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Load and write raw Gen I Pokémon save files (.sav) as byte buffers.
//   - This module ONLY handles disk I/O.
//
//  Owns:
//   - Reading an entire file into memory (std::vector<uint8_t>)
//   - Writing a byte buffer to disk (always to a new path)
//   - Creating a "(BACKUP) <original>.sav" copy of an input file
//   - Creating an "(EDITED) <original>.sav" output path
//
//  Does NOT:
//   - Know Pokémon offsets/banks/save structure
//   - Compute checksums
//   - Decode/encode game data
//

#include <cstdint>
#include <string>
#include <vector>

namespace savegenie {

class FileManipulation {
public:
    using Byte  = std::uint8_t;
    using Bytes = std::vector<Byte>;

    // Load an entire file from disk into a byte buffer.
    // Throws std::runtime_error on failure.
    static Bytes LoadFile(const std::string& path);

    // Write an entire byte buffer to disk.
    // Throws std::runtime_error on failure.
    static void WriteFile(const std::string& path, const Bytes& bytes);

    // Create a "(BACKUP) <original filename>" copy in the same directory.
    // Example:
    //   "Pokemon - Red Version.sav"
    // becomes
    //   "(BACKUP) Pokemon - Red Version.sav"
    // Returns the backup file path.
    // Throws std::runtime_error on failure.
    static std::string BackupFile(const std::string& path);

    // Create the "(EDITED) <original filename>" path in the same directory.
    // This does NOT write the file — it only generates the name.
    // Example:
    //   "Pokemon - Red Version.sav"
    // becomes
    //   "(EDITED) Pokemon - Red Version.sav"
    static std::string MakeEditedPath(const std::string& path);

    // Derive a backup path from an input path.
    // Internal helper used by BackupFile.
    static std::string MakeBackupPath(const std::string& path);
};

} // namespace savegenie

#endif /* FileManipulation_hpp */
