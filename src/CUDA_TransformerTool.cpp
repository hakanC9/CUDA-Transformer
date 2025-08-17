#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"


#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <regex>
#include <thread>
#include <map>
#include <sstream>
#include <fstream>


#include "Transformer/CUDA_TransformerTool.h"
#include "Helper/AccuracyEvaluator.h"
#include "Transformer/CUDA_Analyze_FrontendActionFactory.h"
#include "Transformer/CUDA_Transform_FrontendActionFactory.h"

#include <boost/process.hpp>

#include <unistd.h>
#include <limits.h>

#include "Helper/TimeoutException.h"
#include "Helper/QuickResponse.h"
#include "Helper/FileHandler.h"

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

    // Check if analyze() has already run
    if(optimization_indices.empty()){
        llvm::errs() << "\n Please run analyze() first!\n";
        exit(-1);
    }

    // Reset stop flag for each transformation
    stopFlag = false;


    // Commented-out until proper evaluator implementation

    // if(!evaluator){
    //     llvm::errs() << "\n Please set an evaluator for accuracy!\n";
    //     exit(-1);
    // }


    // Temporarily save the original target project
    FileHandler::saveOriginal(Configurations["project_path"]);

    // Get paths of all cuda files
    std::vector<std::string> cuFiles;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(Configurations["project_path"])) {
        if (entry.is_regular_file() && entry.path().extension() == ".cu") {
            cuFiles.push_back(entry.path().string());
        }
    }

    // Extract each individual optimization
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
    try{
        clang::tooling::ClangTool Tool(*Compilations, cuFiles);

        CUDA_Transform_FrontendActionFactory Factory(optimizationsToApply, optimizationString, stopFlag);

        int result = Tool.run(&Factory);

        if (result != 0){
            FileHandler::revertToOriginal(Configurations["project_path"]);
            return QuickResponse::quickResponse("ClangTool has failed to run!", optimizationString, "-1");
        }

        // Change original files with transformed files
        FileHandler::copyTransformedToOriginal(optimizationString, Configurations["project_path"]);
    }
    catch(TimeoutException& e){
        FileHandler::revertToOriginal(Configurations["project_path"]);
        return QuickResponse::quickResponse(e, optimizationString, "-2");
    }

    // Run build commands in config.txt at target project path
    std::string commandString = "cd \"" + Configurations["project_path"]+ "\" && " + Configurations["commands"];
    try {

        int ret = boost::process::system("/bin/bash", "-c", commandString);

        if (ret != 0) {
            FileHandler::revertToOriginal(Configurations["project_path"]);
            return QuickResponse::quickResponse("Command has failed to run!", optimizationString, "-3");
        }

        std::cout << "All commands executed successfully.\n";
    } catch (const std::exception& e) {
        
        return QuickResponse::quickResponse(e, optimizationString, "-4");
    }
    
    // Run target projects executable
    std::vector<std::string> outputs = run(optimizationString);
    FileHandler::revertToOriginal(Configurations["project_path"]);
    return outputs;
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
std::vector<std::string> CUDA_TransformerTool::run(std::string& optimizationString) {

    std::cout << "\nExecuting the depo...\n";

    // Adding run options are commented-out until proper bug fix
    std::string command = "sudo ./energy.sh "+ Configurations["executable_path"]+ " " + Configurations["run_options"];

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


void CUDA_TransformerTool::runTimer(std::chrono::seconds timeout){
    std::cout << "\nTimeout started\n";
    std::this_thread::sleep_for(timeout);
    stopFlag = true;
}
