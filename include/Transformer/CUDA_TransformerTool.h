#ifndef CUDA_TRANSFORMERTOOL
#define CUDA_TRANSFORMERTOOL

#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"

#include <memory>
#include <regex>
#include <string>
#include <vector>
#include "iostream"
#include <sstream>
#include <filesystem>

class CUDA_TransformerTool {
public:

    CUDA_TransformerTool();

    std::vector<std::string> analyze(std::vector<std::string> sourcePaths);

    std::vector<std::string> analyze(std::string &directoryPath);

    std::vector<std::string> transform(std::vector<std::string> sourcePaths, std::string resultDir, std::string opitimizationString, std::string indicesString, 
        std::string compileOptions, std::string executable);

    std::vector<std::string> transform(std::string &directoryPath, std::string resultDir, std::string opitimizationString, std::string indicesString, 
        std::string compileOptions, std::string executable);    

    std::vector<std::string> transformSingle(std::string sourcePath, std::string resultDir, std::string opitimizationString, 
        std::string compileOptions, std::string executable);

    void printConfigs();

    std::vector<std::string> transform();

private:

    std::unique_ptr<clang::tooling::CompilationDatabase> Compilations;

    std::map<std::string, std::string> Configurations;

    void compile(const std::string compileOptions, const std::string &directoryPath){
        
        std::vector<std::string> cuFiles;

        for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cu") {
                cuFiles.push_back(entry.path().string());
            }
        }
        for(auto cuFile : cuFiles){
            // change .cu to .o
            std::string executableName = cuFile.substr(0, cuFile.size() - 3) + ".o";
            
            // create the command
            std::string command = "nvcc " + cuFile + " -o " + executableName + " " + compileOptions;
            
            
            // run the command
            int result = std::system(command.c_str());
        
            if (result != 0) {
                std::cerr << "Compilation failed with code: " << result << std::endl;
            } else {
                std::cout << "Compilation succeeded. Executable: " << executableName << std::endl;
            }
        }
    }

    std::string run(std::string executable){
        
        std::string command = "./temp_results/" + executable;

        std::array<char, 128> buffer;
        std::stringstream executionResult;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

        if (!pipe)
        {
            std::cerr << "Failed to run command.\n";
            return "null";
        }

        // Read the output of the command
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            executionResult << buffer.data();
        }
        return executionResult.str();
    }

    std::string runDepoTool(std::string executable){
        
        std::string command = "sudo ../energy1.sh " + executable;

        std::array<char, 128> buffer;
        std::stringstream executionResult;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

        if (!pipe)
        {
            std::cerr << "Failed to run command.\n";
            return "null";
        }

        // Read the output of the command
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            executionResult << buffer.data();
        }
        return executionResult.str();
    }

    std::vector<std::string> getDepoResults(std::string depoOutputString){

        // Catch the floating point part of the string
        std::regex numberRegex(R"((\d+(\.\d+)?))");
        std::smatch match;
        std::vector<std::string> values;

        auto begin = depoOutputString.cbegin();
        auto end = depoOutputString.cend();

        while (std::regex_search(begin, end, match, numberRegex)) {
            values.push_back(match[1]);
            begin = match.suffix().first;
        }

        // If vector is empty (moslty means depo tool calculations are 0), return 0 0 0 values
        while (values.size() < 3) {
            values.push_back("0.0");
        }

        return values;
    }

};

#endif