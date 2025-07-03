#include "clang/Tooling/CompilationDatabase.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>

#include "Transformer/CUDA_TransformerTool.h"
#include "Transformer/CUDA_Analyze_FrontendActionFactory.h"
#include "Transformer/CUDA_Transform_FrontendActionFactory.h"


/**
 * @brief A function to read config.txt file
 * @return std::map<std::string, std::string> with keys as lhs and values as rhs
*/
std::map<std::string, std::string> readConfig() {
    std::map<std::string, std::string> configMap;
    std::ifstream file("../config.txt");
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                configMap[key] = value;
            }
        }
    }
    return configMap;
}


/**
 * @brief A helper function that parses config.txt files compile_options key
 * @param line   A string with semicolumn seperated values
 * @return std::string whitespace seperated compile options
*/
std::string parseCompileOptions(std::string line){

    std::stringstream ss(line);

    std::string options;
    std::string temp;

    while(std::getline(ss, temp, ';')){
        options += " " + temp;
    }

    return options;
}

/**
 * @brief A helper function that parses config.txt files compile_options key
 * @param line   A string with semicolumn seperated values
 * @return std::string whitespace seperated compile options
*/
std::vector<std::string>  parseCompilationDatabaseOptions(std::string line){

    std::stringstream ss(line);

    std::vector<std::string>  options;
    std::string temp;

    while(std::getline(ss, temp, ';')){
        options.push_back(temp);
    }

    return options;
}


/**
 * @brief A helper function that parses config.txt files includes key
 * @param line   A string with semicolumn seperated values
 * @return std::vector<std::string> all parsed include options with -I prefix
*/
std::vector<std::string>  parseIncludes(std::string line){

    std::stringstream ss(line);

    std::vector<std::string> include;
    std::string temp;

    while(std::getline(ss, temp, ';')){
        include.push_back("-I" + temp);
    }

    return include;
}


CUDA_TransformerTool::CUDA_TransformerTool() {

    auto config = readConfig();

    std::vector<std::string> compileFlags;

    compileFlags.push_back("-resource-dir=" + config["resource_dir"]);

    for(auto inc : parseCompilationDatabaseOptions(config["compilation_database_options"])){
        compileFlags.push_back(inc);
    }    

    for(auto inc : parseIncludes(config["includes"])){
        compileFlags.push_back(inc);
    }

    Configurations = config;

    Compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", compileFlags);
}


/**
 * @brief A function to analyze given .cu file to obtain which optimizations can be applied.
 * 
 * @param sourcePaths   A vector containing paths to target .cu files (example: /home/Desktop/project/example.cu)
 * @return              A vector containing optimizations to apply in order given with parameter (example: {347, 34667})
 */
std::vector<std::string> CUDA_TransformerTool::analyze(std::vector<std::string> sourcePaths){

    clang::tooling::ClangTool Tool(*Compilations, sourcePaths);

    CUDA_Analyze_FrontendActionFactory Factory;

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running analyze!\n";
        exit(-1);
    }

    return Factory.getOptimizationPossibilities();
}


/**
 * @brief A function to analyze .cu files in the target project directory to obtain which optimizations can be applied.
 * 
 * @return A vector containing optimizations to apply in order given with parameter (example: {347, 34667})
 */
std::vector<std::string> CUDA_TransformerTool::analyze(){

    std::vector<std::string> cuFiles;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(Configurations["project_path"])) {
        if (entry.is_regular_file() && entry.path().extension() == ".cu") {
            cuFiles.push_back(entry.path().string());
        }
    }

    return CUDA_TransformerTool::analyze(cuFiles);
}


/**
 * @brief A function to transform, compile, execute and collect metrics with depo tool. Using CUDA_TransformerTool::analyze before this function
 * to determine which optimization choices to use is encouraged.
 *
 * @param optimizationString    A string of optimization to apply for each target file (example: 11111111)
 * @param optimizationIndices   A string of "-" seperated values that corresponds each individual optimization strings size 
                                (example: 5-3 corresponds to 11111 and 111 for the optimizationString=11111111)
 *
 * @return std::vector<string> : 
 * 0 -> Execution Result: Unknown
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
*/
std::vector<std::string> CUDA_TransformerTool::transform(std::string optimizationString, std::string optimizationIndices){


    // Get paths of all cuda files

    std::vector<std::string> cuFiles;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(Configurations["project_path"])) {
        if (entry.is_regular_file() && entry.path().extension() == ".cu") {
            cuFiles.push_back(entry.path().string());
        }
    }


    // Extract each individual optimizations

    std::stringstream ss(optimizationIndices);

    std::vector<int> indexVec;
    std::string temp;

    while(std::getline(ss, temp, '-')){
        indexVec.push_back(std::stoi(temp));
    }

    std::vector<std::string> optimizationsToApply;
    int start = 0;

    for (int index : indexVec) {
        optimizationsToApply.push_back(optimizationString.substr(start,index));
        start += index;
    }


    // Run the tool

    clang::tooling::ClangTool Tool(*Compilations, cuFiles);

    CUDA_Transform_FrontendActionFactory Factory(optimizationsToApply);

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running transform!\n";
        exit(-1);
    }

    // Compile, run and use depo tool on transformed files
    
    compile(parseCompileOptions(Configurations["compile_options"]), "temp_results");

    std::vector<std::string> results;

    std::string executionResult = run(Configurations["executable"], Configurations["run_options"]);
    std::string depoExecutionResult = runDepoTool(Configurations["executable"]);

    std::vector<std::string> depoResult = getDepoResults(depoExecutionResult);

    results.push_back(executionResult);
    results.insert(results.end(), depoResult.begin(), depoResult.end());

    return results;
}