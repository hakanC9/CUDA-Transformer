#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>


#include "Transformer/CUDA_TransformerTool.h"
#include "Helper/AccuracyEvaluator.h"
#include "Transformer/CUDA_Analyze_FrontendActionFactory.h"
#include "Transformer/CUDA_Transform_FrontendActionFactory.h"

#include <boost/process.hpp>

CUDA_TransformerTool::CUDA_TransformerTool() {

    Configurations = parser.readConfig();

    std::vector<std::string> compileFlags;

    compileFlags.push_back("-resource-dir=" + Configurations["resource_dir"]);

    for(auto inc : parser.parseWhitespace(Configurations["compilation_database_options"])){
        compileFlags.push_back(inc);
    }    

    for(auto inc : parser.parseWhitespace(Configurations["includes"])){
        compileFlags.push_back(inc);
    }

    Compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", compileFlags);
}


/**
 * @brief A function to analyze .cu files in the target project directory to obtain which optimizations can be applied.
 * 
 * @return A vector containing optimizations to apply in order given with parameter (example: {347, 34667})
 */
std::vector<std::string> CUDA_TransformerTool::analyze(){

    std::vector<std::string> sourcePaths;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(Configurations["project_path"])) {
        if (entry.is_regular_file() && entry.path().extension() == ".cu") {
            sourcePaths.push_back(entry.path().string());
        }
    }

    // reset index
    optimization_indices = "";

    // ready the tool and run it
    clang::tooling::ClangTool Tool(*Compilations, sourcePaths);

    CUDA_Analyze_FrontendActionFactory Factory;

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running analyze!\n";
        exit(-1);
    }

    // calculate the optimization string lengths for eaach analyzed file
    std::vector<std::string> optimizationPossibilities = Factory.getOptimizationPossibilities();

    for(auto result : optimizationPossibilities){
        optimization_indices += std::to_string(result.size()) +  "-";
    }

    // clear the last '-'
    optimization_indices.pop_back();

    return Factory.getOptimizationPossibilities();
}


/**
 * @brief A function to transform, compile, execute and collect metrics with depo tool. Using CUDA_TransformerTool::analyze before this function
 * to determine which optimization choices to use is encouraged.
 *
 * @param optimizationString    A string of optimization to apply for each target file (example: 11111111)
 *
 * @return std::vector<string> : 
 * 0 -> Execution Result: Varying
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
*/
std::vector<std::string> CUDA_TransformerTool::transform(std::string optimizationString){


    if(optimization_indices.empty()){
        llvm::errs() << "\n Please run analyze() first!\n";
        exit(-1);
    }

    // Commented-out until proper evaluator implementation

    // if(!evaluator){
    //     llvm::errs() << "\n Please set an evaluator for accuracy!\n";
    //     exit(-1);
    // }


    // Temporarily save the original target project
    saveOriginal();

    // Get paths of all cuda files
    std::vector<std::string> cuFiles;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(Configurations["project_path"])) {
        if (entry.is_regular_file() && entry.path().extension() == ".cu") {
            cuFiles.push_back(entry.path().string());
        }
    }

    // Extract each individual optimizations
    std::stringstream ss(optimization_indices);

    std::vector<int> indexVec;
    std::string temp;

    while(std::getline(ss, temp, '-')){
        indexVec.push_back(std::stoi(temp));
    }

    std::vector<std::string> optimizationsToApply;
    int start = 0;

    for (int index : indexVec) {
        if(index == 0){
            optimizationsToApply.push_back("");
        }
        else{
            optimizationsToApply.push_back(optimizationString.substr(start,index));
            start += index;            
        }
    }


    // Run the tool
    clang::tooling::ClangTool Tool(*Compilations, cuFiles);

    CUDA_Transform_FrontendActionFactory Factory(optimizationsToApply, optimizationString);

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running transform!\n";
        exit(-1);
    }

    // Change original files with transformed files
    copyTransformedToOriginal(optimizationString);


    std::string commandString = "cd \"" + Configurations["project_path"]+ "\" && " + Configurations["commands"];
    try {

        int ret = boost::process::system("/bin/bash", "-c", commandString);

        if (ret != 0) {
            std::cerr << "[Error] Command sequence failed with code: " << ret << "\n";
            exit(-1);
        }

        std::cout << "All commands executed successfully.\n";
    } catch (const std::exception& e) {
        std::cerr << "[Exception] " << e.what() << "\n";
        exit(-1);
    }
    
    // Change transformed files with original files
    revertToOriginal();
    
    return run(optimizationString);
}


void CUDA_TransformerTool::setAccuracyEvaluator(std::unique_ptr<AccuracyEvaluator> eval){
    evaluator = std::move(eval);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
 * @brief A function to run the depo tool with target executable for calculating power metrics
 * 
 * @param optimizationString    Optimization string to run the target executable under temp_results/{optimizationString}
 * @return std::vector<string> : 
 * 0 -> Execution Result: Varying
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
 */
std::vector<std::string> CUDA_TransformerTool::run(std::string& optimizationString){
    
    std::cout << "\nExecuting the depo...\n";

    // Adding run options are commented-out until proper bug fix
    std::string command = "sudo ../energy.sh "+ Configurations["executable_path"] + " " ;//+ Configurations["run_options"];

    std::array<char, 128> buffer;
    std::stringstream executionResult;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe)
    {
        std::cerr << "Failed to run command.\n";
        exit(-1);
    }

    // Read the output of the command
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        executionResult << buffer.data();
    }


    std::vector<std::string> depoResult = getDepoResults(executionResult.str());
    std::string programExecutionResult = readProgramOutput();
    
    std::vector<std::string> results;
    
    results.push_back(programExecutionResult);

    results.insert(results.end(), depoResult.begin(), depoResult.end());

    return results;
}


/**
 * @brief A function that overwrites target projects files with transformed files
 */
void CUDA_TransformerTool::copyTransformedToOriginal(std::string& optimizationString) {

    std::filesystem::copy(
        "temp_results/" + optimizationString,
        Configurations["project_path"],
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive
    );
}


/**
 * @brief A function that temporarily saves the target projects unchanged files
 */
void CUDA_TransformerTool::saveOriginal(){

    try {
        if (std::filesystem::exists("temp_save")) {
            std::filesystem::remove_all("temp_save");
        }

        std::filesystem::copy(
            Configurations["project_path"],
            "temp_save",
            std::filesystem::copy_options::recursive
        );

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error copying original files: " << e.what() << std::endl;
    }
}


/**
 * @brief A function that reverts the changes on the target project
 */
void CUDA_TransformerTool::revertToOriginal(){

    std::filesystem::copy(
        "temp_save",
        Configurations["project_path"],
        std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive
    );

    std::filesystem::remove_all("temp_save");
}


/**
 * @brief A function to parse DEPO Tool's output
 * 
 * @param depoOutputString    DEPO Tool's ouput
 *
 * @return std::vector<string> : 
 * 0 -> Energy Consumption (E): Joule
 * 1 -> Execution Time (t): second
 * 2 -> Power Consumption (P): Watt
 */
std::vector<std::string> CUDA_TransformerTool::getDepoResults(std::string depoOutputString) {
    std::vector<std::string> values;
    std::smatch match;

    std::regex energyRegex(R"(Energy Consumption.*?:\s*([0-9]*\.?[0-9]+))");
    std::regex timeRegex(R"(Execution Time.*?:\s*([0-9]*\.?[0-9]+))");
    std::regex powerRegex(R"(Power Consumption.*?:\s*([0-9]*\.?[0-9]+))");

    if (std::regex_search(depoOutputString, match, energyRegex))
        values.push_back(match[1]);
    else
        values.push_back("0.0");

    if (std::regex_search(depoOutputString, match, timeRegex))
        values.push_back(match[1]);
    else
        values.push_back("0.0");

    if (std::regex_search(depoOutputString, match, powerRegex))
        values.push_back(match[1]);
    else
        values.push_back("0.0");

    return values;
}


/**
 * @brief A function that reads EP_stdout.txt file generated from DEPO Tool for target projects' output
 * 
 * @return std::string Read output
 */
std::string CUDA_TransformerTool::readProgramOutput() {

    std::ifstream file("EP_stdout.txt");
    
    if (!file.is_open()) {
        return "(EP_stdout.txt not found)";
    }

    std::vector<std::string> lines;
    std::string line;
    
    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // skip debug line
    std::stringstream buffer;
    for (size_t i = 0; i + 1 < lines.size(); i++) {
        buffer << lines[i] << '\n';
    }

    return buffer.str();
}

