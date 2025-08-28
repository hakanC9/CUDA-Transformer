#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"


#include <chrono>
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

    // calculate the optimization string lengths for each analyzed file
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
    
    // Extract each individual optimization to apply
    std::vector<std::string> optimizationsToApply = parser.extractOptimizations(optimization_indices, optimizationString);


    // Run the tool

    clang::tooling::ClangTool Tool(*Compilations, cuFiles);

    CUDA_Transform_FrontendActionFactory Factory(optimizationsToApply, optimizationString);

    int result = Tool.run(&Factory);

    if (result != 0){
        FileHandler::revertToOriginal(Configurations["project_path"]);
        return QuickResponse::quickResponse("ClangTool has failed to run!", optimizationString, "-1");
    }

    // Change original files with transformed files
    FileHandler::copyTransformedToOriginal(optimizationString, Configurations["project_path"]);

    // Run build commands in config.txt at target project's path
    std::string commandString = "cd \"" + Configurations["project_path"]+ "\" && " + Configurations["commands"];
    int ret = boost::process::system("/bin/bash", "-c", commandString);

    // Exception lazım

    if (ret != 0) {
        FileHandler::revertToOriginal(Configurations["project_path"]);
        return QuickResponse::quickResponse("Build command has failed to run!", optimizationString, "-3");
    }
    
    // Start the timer
    Timer::runTimer(std::chrono::seconds(std::stoi(Configurations["timeout"])), stopFlag);

    // Run target project's executable
    std::vector<std::string> outputs = run(optimizationString);
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

    std::cout << "Execution started\n";
    try{
        std::atomic<bool> timedOut{false};
        std::string command = "./energy.sh";
        std::vector<std::string> args = { Configurations["executable_path"], Configurations["run_options"] };

        boost::process::ipstream pipe_stream;
        boost::process::child c(command, boost::process::args(args), boost::process::std_out > pipe_stream);

        std::stringstream executionResult;
        std::string line;

        // Thread to monitor stopFlag
        std::thread killer([c_ptr = &c, &timedOut, this]() {
            while (c_ptr->running()) {
                if (stopFlag.load()) {
                    std::cout << "Stopping child process..." << std::endl;
                    c_ptr->terminate();
                    timedOut = true; // signal main thread
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });


        // Read stdout of the child
        while (pipe_stream && std::getline(pipe_stream, line)) {
            executionResult << line << '\n';
        }

        c.wait();        // ensure process has exited
        killer.join();   // join the killer thread

        if (timedOut) throw TimeoutException("Timeout has occurred!");


        std::vector<std::string> depoResult = getDepoResults(executionResult.str());
        std::string programExecutionResult = readProgramOutput();

        std::vector<std::string> results;
        results.push_back(programExecutionResult);
        results.insert(results.end(), depoResult.begin(), depoResult.end());

        FileHandler::revertToOriginal(Configurations["project_path"]);
        return results;
    }
    catch (const TimeoutException& e){
        FileHandler::revertToOriginal(Configurations["project_path"]);
        return QuickResponse::quickResponse(
            std::string("energy.sh has failed to run! Error: ") + e.what(),
            optimizationString, "-2");
    }
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
 * @brief A function that reads EP_stdout.txt file generated from DEPO Tool for target project's output
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


