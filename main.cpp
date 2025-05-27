#include "Transformer/CUDA_TransformerTool.h"
#include <string>
#include <vector>

/*
* Example usage of the tool
*/
int main(int argc, char**arv){

    CUDA_TransformerTool TransformerToolTest("/home/pars/Desktop/llvm-project/build/lib/clang/21");
    
    
    const std::string &filePath1 = "/home/pars/Desktop/project_example/example.cu";
    const std::string &filePath2 = "/home/pars/Desktop/project_example/kernel.cu";
    std::string directoryPath = "/home/pars/Desktop/project_example/";


    std::vector<std::string> analysis = TransformerToolTest.analyze(directoryPath);

    std::cout << "Analysis Output: \n";
    for(auto a : analysis){
        std::cout << a << "\n";
    }

    std::string resultDir = "RESULTS";
    std::string optimizationString = "11111111";
    std::string indicesString = "4";
    std::string compileOptions = " ";
    std::string executable = resultDir + "/11111.o";  
    
    std::vector<std::string> output = TransformerToolTest.transform({filePath1, filePath2}, resultDir, 
        optimizationString, indicesString, compileOptions, executable);
    
    std::cout << "Tool Output: \n";
    for(auto out:output){
        std::cout << out << "\n";
    }
}