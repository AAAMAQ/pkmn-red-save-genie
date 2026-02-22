//
//  main.cpp
//  Pkmn Red Save Genie
//
//  Purpose:
//   - Reader-only test harness.
//   - Flow: Load save -> backup -> validate -> dump readable summary.
//

#include <iostream>
#include <stdexcept>

#include "FileManipulation.hpp"
#include "SaveStructure.hpp"
#include "ReadOnlyData.hpp"

int main() {
    using namespace savegenie;

    const std::string inputPath =
        "Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav";

    try {
        // 1) Create backup (safe)
        const std::string backupPath = FileManipulation::BackupFile(inputPath);

        // 2) Load bytes
        FileManipulation::Bytes bytes = FileManipulation::LoadFile(inputPath);

        // 3) Wrap in SaveBuffer (safe access)
        SaveBuffer save(std::move(bytes));

        // 4) Validate basic properties
        std::cout << "Input:  " << inputPath << "\n";
        std::cout << "Backup: " << backupPath << "\n";
        std::cout << "Size:   0x" << std::hex << save.Size() << std::dec << " bytes\n";

        if (!SaveValidator::HasExpectedSize(save)) {
            std::cout << "[WARN] Save size is not 0x8000 (32KB). This may not be a Gen I save.\n";
        }

        std::cout << "Main Checksum: "
                  << (SaveValidator::HasValidMainChecksum(save) ? "VALID" : "INVALID")
                  << "\n\n";

        // 5) Dump readable summary
        ReadOnlyData reader(save);
        std::cout << reader.DumpFullSummary() << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }
}
