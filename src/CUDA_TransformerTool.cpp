#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

#include "Transformer/CUDA_TransformerTool.h"
#include "Transformer/CUDA_FrontendActionFactory.h"

/**
 * @param resourceDir directory path of clang (example: /path-to-llvm/build/lib/clang/21)
 */
CUDA_TransformerTool::CUDA_TransformerTool(std::string resourceDir) {

    this->resourceDir = "-resource-dir=" + resourceDir;
    std::vector<std::string> compileFlags = {
        "-std=c++17",
        "-x", "cuda",
        this->resourceDir
    };

    Compilations = std::make_unique<clang::tooling::FixedCompilationDatabase>(".", compileFlags);
    optimizationPossibilities = std::make_shared<std::string>("null");
    outputFilePath = std::make_shared<std::string>("null");
}

/**
 * @brief A function to transform, compile, execute and collect metrics with depo tool. Using CUDA_TransformerTool::analyzeOptimizationChoices before this function
 * to determine which optimization choices to use is encouraged.
 * 
 * @param filePath          Path to target .cu file (example: /home/Desktop/project/example.cu)
 * @param optChoices        Optimizations to apply (example: 0231110)
 * @param resultDir         Name of the result directory that will be created in current path
 * @param compileOptions    Additional compiler options to pass to nvcc
 * @return std::vector<double> 
 * 0 -> Energy Consumption (E): Joule
 * 1 -> Execution Time (t): second
 * 2 -> Power Consumption (P): Watt
*/
std::vector<double> CUDA_TransformerTool::evaluate(const std::string filePath, const std::string optChoices, std::string resultDir, std::string compileOptions) {


    std::vector<std::string> sources = {filePath};
    clang::tooling::ClangTool Tool(*Compilations, sources);

    CUDA_FrontendActionFactory Factory(false, optChoices, optimizationPossibilities, outputFilePath, resultDir);

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running evaluate!\n";
    }

    CUDA_TransformerTool::compile(compileOptions);
    CUDA_TransformerTool::run();
    
    std::string depoOutputString = CUDA_TransformerTool::runDepoTool();
    return CUDA_TransformerTool::getDepoResults(depoOutputString);

}

/**
 * @brief A function to analyze given .cu file to obtain which optimizations can be applied.
 * 
 * @param filePath      Path to target .cu file (example: /home/Desktop/project/example.cu)
 * @return std::string Optimizations to apply (example: 347)
 */
std::string CUDA_TransformerTool::analyzeOptimizationChoices(const std::string filePath){

    std::vector<std::string> sources = {filePath};
    clang::tooling::ClangTool Tool(*Compilations, sources);

    CUDA_FrontendActionFactory Factory(true, " ", optimizationPossibilities, outputFilePath,  " ");

    int result = Tool.run(&Factory);

    if (result != 0){
        llvm::errs() << "Error running analyze!\n";
        return "null";
    }

    return *CUDA_TransformerTool::getOptimizationPossibilities();
}
