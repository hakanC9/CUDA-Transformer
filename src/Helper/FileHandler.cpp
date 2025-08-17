#include "Helper/FileHandler.h"
#include <filesystem>
#include <iostream>

/**
 * @brief A function that overwrites target projects files with transformed files
 *
 * @param optimizationString  A string of optimization to apply for each target file (example: 11111111)
 * @param path                Path to target project
 *
 */
void FileHandler::copyTransformedToOriginal(const std::string& optimizationString, const std::string& path){
    try{
        std::filesystem::copy(
            "temp_results/" + optimizationString,
            path,
            std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive
        );
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error copying transformed files to target path: " << e.what() << std::endl;
        exit(-1);
    }
}


/**
 * @brief A function that temporarily saves the target projects unchanged files
 *
 * @param path  Path to target project
 *
 */
void FileHandler::saveOriginal(const std::string& path){

    try {
        if (std::filesystem::exists("temp_save")) {
            std::filesystem::remove_all("temp_save");
        }

        std::filesystem::copy(
            path,
            "temp_save",
            std::filesystem::copy_options::recursive
        );

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error copying original files: " << e.what() << std::endl;
        exit(-1);
    }
}

/**
 * @brief A function that reverts the changes on the target project
 *
 * @param path  Path to target project
 *
 */
void FileHandler::revertToOriginal(const std::string& path){

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::filesystem::remove_all(entry.path());
    }

    std::filesystem::copy(
        "temp_save",
        path,
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive
    );

    std::filesystem::remove_all("temp_save");
    std::cout << "\nFiles are reverted back to original\n";
}