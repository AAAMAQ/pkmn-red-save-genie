
//
//  FileManipulation.cpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Implementation of FileManipulation: disk I/O only.
//   - Loads and writes raw .sav files as byte buffers.
//
//  Notes:
//   - This module intentionally contains NO Pokémon-specific logic.
//   - All errors are reported via std::runtime_error.
//

#include "FileManipulation.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace savegenie {

FileManipulation::Bytes FileManipulation::LoadFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) {
        throw std::runtime_error("LoadFile failed: could not open input file: " + path);
    }

    const std::streamsize size = in.tellg();
    if (size < 0) {
        throw std::runtime_error("LoadFile failed: could not determine file size: " + path);
    }

    Bytes bytes(static_cast<std::size_t>(size));
    in.seekg(0, std::ios::beg);

    if (!bytes.empty()) {
        in.read(reinterpret_cast<char*>(bytes.data()), size);
        if (!in) {
            throw std::runtime_error("LoadFile failed: read error for file: " + path);
        }
    }

    return bytes;
}

void FileManipulation::WriteFile(const std::string& path, const Bytes& bytes) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("WriteFile failed: could not open output file: " + path);
    }

    if (!bytes.empty()) {
        out.write(reinterpret_cast<const char*>(bytes.data()),
                  static_cast<std::streamsize>(bytes.size()));
        if (!out) {
            throw std::runtime_error("WriteFile failed: write error for file: " + path);
        }
    }

    out.flush();
    if (!out) {
        throw std::runtime_error("WriteFile failed: flush error for file: " + path);
    }
}

std::string FileManipulation::MakeBackupPath(const std::string& path) {
    namespace fs = std::filesystem;
    fs::path p(path);
    fs::path dir = p.parent_path();
    std::string filename = p.filename().string();
    fs::path out = dir / ("(BACKUP) " + filename);
    return out.string();
}

std::string FileManipulation::MakeEditedPath(const std::string& path) {
    namespace fs = std::filesystem;
    fs::path p(path);
    fs::path dir = p.parent_path();
    std::string filename = p.filename().string();
    fs::path out = dir / ("(EDITED) " + filename);
    return out.string();
}

std::string FileManipulation::BackupFile(const std::string& path) {
    namespace fs = std::filesystem;

    const std::string backupPath = MakeBackupPath(path);

    std::error_code ec;
    // If backup already exists, keep it (don't overwrite the user's old backup).
    if (fs::exists(backupPath, ec) && !ec) {
        return backupPath;
    }

    // Copy file data. overwrite_existing is false by design.
    fs::copy_file(path, backupPath, fs::copy_options::none, ec);
    if (ec) {
        throw std::runtime_error(
            "BackupFile failed: could not create backup '" + backupPath + "' from '" + path + "' (" +
            ec.message() + ")");
    }

    return backupPath;
}

} // namespace savegenie
