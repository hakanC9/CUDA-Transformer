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

class CUDA_TransformerTool {
public:

    CUDA_TransformerTool(std::string resourceDir);

    std::vector<double> evaluate(const std::string filePath, const std::string optChoices, std::string resultDir, std::string compileOptions);

    std::string analyzeOptimizationChoices(const std::string filePath);

    std::shared_ptr<std::string> getOptimizationPossibilities() const {return optimizationPossibilities;}

    std::shared_ptr<std::string> getoutputFilePath() const {return outputFilePath;}


private:
    std::string resourceDir;
    std::string filePath;
    bool analyzerMode;
    std::unique_ptr<clang::tooling::CompilationDatabase> Compilations;
    std::shared_ptr<std::string> optimizationPossibilities;
    std::shared_ptr<std::string>  outputFilePath;

    std::string executable;

    void compile(const std::string compileOptions){

        // change .cu to .o
        std::string outputFile = *outputFilePath; 
        std::string executableName = outputFile.substr(0, outputFile.size() - 3) + ".o";
        
        executable = executableName;

        // create the command
        std::string command = "nvcc " + outputFile + " -o " + executableName + " " + compileOptions;
        
        //std::cout << "Compiling with command: " << command << std::endl;
        
        // run the command
        int result = std::system(command.c_str());
    
        if (result != 0) {
            std::cerr << "Compilation failed with code: " << result << std::endl;
        } else {
            std::cout << "Compilation succeeded. Executable: " << executableName << std::endl;
        }
    }

    std::string run(){
        
        std::string command = "./" + executable;

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

    std::string runDepoTool(){

        // std::string password = "pars";
        // std::string command = "echo " + password + " | sudo ../energy1.sh " + executable;
        
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

    std::vector<double> getDepoResults(std::string depoOutputString){

        // Catch the floating point part of the string
        std::regex numberRegex(R"((\d+(\.\d+)?))");
        std::smatch match;
        std::vector<double> values;

        auto begin = depoOutputString.cbegin();
        auto end = depoOutputString.cend();

        while (std::regex_search(begin, end, match, numberRegex)) {
            values.push_back(std::stod(match[1]));
            begin = match.suffix().first;
        }

        // If vector is empty (moslty means depo tool calculations are 0), return 0 0 0 values
        while (values.size() < 3) {
            values.push_back(0.0);
        }

        return values;
    }
};

#endif