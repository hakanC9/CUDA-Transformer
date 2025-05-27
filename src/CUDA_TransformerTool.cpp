#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>

#include "Transformer/CUDA_TransformerTool.h"
#include "Transformer/CUDA_Analyze_FrontendActionFactory.h"
#include "Transformer/CUDA_Transform_FrontendActionFactory.h"

/**
 * @param resourceDir directory path of clang (example: /path-to-llvm/build/lib/clang/21)
 */
CUDA_TransformerTool::CUDA_TransformerTool(std::string resourceDir) {

    std::vector<std::string> compileFlags = {
        "-std=c++17",
        "-x", "cuda",
        "-resource-dir=" + resourceDir
    };

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
 * @brief A function to analyze given .cu file to obtain which optimizations can be applied.
 * 
 * @param sourcePaths   A vector containing paths to target .cu files (example: /home/Desktop/project/example.cu)
 * @return              A vector containing optimizations to apply in order given with parameter (example: {347, 34667})
 */
std::vector<std::string> CUDA_TransformerTool::analyze(std::string &directoryPath){

    std::vector<std::string> cuFiles;

    for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
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
 * @param sourcePaths                A vector containing target .cu file paths (example: /home/Desktop/project/example.cu)
 * @param resultDir                  Name of the result directory that will be created in current path
 * @param opitimizationString        Optimizations to apply (example: 0231110)
 * @param indicesString              Semi-column seperated values for slicing optimization string (example: "4" slices "11111021" as {"1111", "021"})
 * @param compileOptions             Additional compiler options to pass to nvcc
 * @param executable                 Absoulte path for file to be executed.
 * @return std::vector<string> : 
 * 0 -> Execution Result: Unknown
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
*/
std::vector<std::string> CUDA_TransformerTool::transform(std::vector<std::string> sourcePaths, std::string resultDir, 
    std::string opitimizationString, std::string indicesString, std::string compileOptions, std::string executable){

    std::vector<int> indices;
    std::stringstream ss(indicesString);
    std::string token;

    while (std::getline(ss, token, ';')) {
        indices.push_back(std::stoi(token));
    }

    std::vector<std::string> optimizationsToApply;
    int start = 0;

    for (int index : indices) {
        int length = index - start + 1; 
        optimizationsToApply.push_back(opitimizationString.substr(start, length));
        start = index + 1;
    }

    if (start < opitimizationString.size()) {
        optimizationsToApply.push_back(opitimizationString.substr(start));
    }

    clang::tooling::ClangTool Tool(*Compilations, sourcePaths);

    CUDA_Transform_FrontendActionFactory Factory(resultDir, optimizationsToApply);

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running transform!\n";
        exit(-1);
    }

    compile(compileOptions, resultDir);

    std::vector<std::string> results;

    std::string executionResult = run(executable);
    std::string depoExecutionResult = runDepoTool(executable);

    std::vector<std::string> depoResult = getDepoResults(depoExecutionResult);

    results.push_back(executionResult);
    results.insert(results.end(), depoResult.begin(), depoResult.end());

    return results;
}


/**
 * @brief A function to transform, compile, execute and collect metrics with depo tool. Using CUDA_TransformerTool::analyze before this function
 * to determine which optimization choices to use is encouraged.
 * 
 * @param sourcePath                Path to target .cu file (example: /home/Desktop/project/example.cu)
 * @param optimizationString        Optimizations to apply (example: 0231110)
 * @param resultDir                 Name of the result directory that will be created in current path
 * @param compileOptions            Additional compiler options to pass to nvcc
 * @param executable                Absoulte path for file to be executed.
 * @return std::vector<string> : 
 * 0 -> Execution Result: Unknown
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
*/
std::vector<std::string> CUDA_TransformerTool::transformSingle(std::string sourcePath, std::string resultDir, std::string optimizationString, 
    std::string compileOptions, std::string executable){
    
    clang::tooling::ClangTool Tool(*Compilations, {sourcePath});

    CUDA_Transform_FrontendActionFactory Factory(resultDir, {optimizationString});

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running transformSingle!\n";
        exit(-1);
    }

    compile(compileOptions, resultDir);

    std::vector<std::string> results;

    std::string executionResult = run(executable);
    std::string depoExecutionResult = runDepoTool(executable);

    std::vector<std::string> depoResult = getDepoResults(depoExecutionResult);

    results.push_back(executionResult);
    results.insert(results.end(), depoResult.begin(), depoResult.end());

    return results;
}


/**
 * @brief A function to transform, compile, execute and collect metrics with depo tool. Using CUDA_TransformerTool::analyze before this function
 * to determine which optimization choices to use is encouraged.
 * 
 * @param directoryPath              Path to target directory (example: /home/Desktop/project/)
 * @param resultDir                  Name of the result directory that will be created in current path
 * @param opitimizationString        Optimizations to apply (example: 0231110)
 * @param indicesString              Semi-column seperated values for slicing optimization string (example: "4" slices "11111021" as {"1111", "021"})
 * @param compileOptions             Additional compiler options to pass to nvcc
 * @param executable                 Absoulte path for file to be executed.
 * @return std::vector<string> : 
 * 0 -> Execution Result: Unknown
 * 1 -> Energy Consumption (E): Joule
 * 2 -> Execution Time (t): second
 * 3 -> Power Consumption (P): Watt
*/
std::vector<std::string> CUDA_TransformerTool::transform(std::string &directoryPath, std::string resultDir, std::string opitimizationString, 
    std::string indicesString, std::string compileOptions, std::string executable){
        
        std::vector<std::string> cuFiles;

        for (const auto &entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cu") {
                cuFiles.push_back(entry.path().string());
            }
        }

        return CUDA_TransformerTool::transform(cuFiles, resultDir, opitimizationString, indicesString, compileOptions, executable);
}

