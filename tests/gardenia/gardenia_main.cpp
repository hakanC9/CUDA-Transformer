#include "Transformer/CUDA_TransformerTool.h"
#include <iostream>

/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;
    
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();
    
    std::cout << "\nANALYZE RESULTS\n";
    for(auto a: analyzeResults){
        std::cout << a << "\n";
    }
    
    std::vector<std::string>results = TransformerToolTest.transform("111");

    std::cout << "\nTRANSFORM RESULTS\n";
    for(auto a: results){
        std::cout << a << "\n";
    }
}




