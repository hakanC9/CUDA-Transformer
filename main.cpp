#include "Transformer/CUDA_TransformerTool.h"
#include <string>

/*
* Example usage of the tool
*/
int main(int argc, char**arv){

    CUDA_TransformerTool TransformerToolTest("/home/pars/Desktop/llvm-project/build/lib/clang/21");
    
    
    const std::string &filePath1 = "/home/pars/Desktop/project_example/example.cu";

    std::string opt1 = TransformerToolTest.analyzeOptimizationChoices(filePath1);
    std::cout << "Optimization choices 1: " << opt1 << "\n";


    std::string resultDir1 = "example";
    std::vector<double> depovalues1;
    depovalues1 = TransformerToolTest.evaluate(filePath1, "02111", resultDir1, " ");

    std::cout << "Energy Consumption: " << depovalues1.at(0) << "\n";
    std::cout << "Execution Time: " << depovalues1.at(1) << "\n";
    std::cout << "Power Consumption: " << depovalues1.at(2) << "\n";


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    const std::string &filePath2 = "/home/pars/Desktop/project_example/kernel.cu";

    std::string opt2 = TransformerToolTest.analyzeOptimizationChoices(filePath2);
    std::cout << "Optimization choices 2: " << opt2 << "\n";

    std::string resultDir2 = "example";
    std::vector<double> depovalues2;
    depovalues2 = TransformerToolTest.evaluate(filePath2, "010", resultDir2, " ");

    std::cout << "Energy Consumption: " << depovalues2.at(0) << "\n";
    std::cout << "Execution Time: " << depovalues2.at(1) << "\n";
    std::cout << "Power Consumption: " << depovalues2.at(2) << "\n";
}