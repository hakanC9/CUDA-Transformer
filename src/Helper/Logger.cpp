#include "Helper/Logger.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>


bool Logger::log(std::vector<std::string> output, std::string& optimizationString) {
    std::filesystem::path dir = "temp_results/" + optimizationString;
    std::error_code ec;

    std::filesystem::create_directories(dir, ec);
    if (ec) {
        llvm::errs() << "Failed to create directory: " << ec.message() << "\n";
        return false;
    }

    std::filesystem::path path = dir / ("log_" + optimizationString + ".txt");
    std::ofstream file(path);

    if (file.is_open()) {
        for (size_t i = 0; i < output.size(); ++i) {
            file << output[i] << "\n";
            if (i != output.size() - 1)
                file << "=============================\n";
        }
        file.close();
        return true;
    } else {
        llvm::errs() << "Failed to write log file: " << path << "\n";
        return false;
    }
}