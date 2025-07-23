#include "Transformer/CUDA_TransformerTool.h"
#include <iostream>
#include <vector>
#include "Helper/Logger.h"


/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;
    Logger logger;
    
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();

    std::cout << "\nANALYZE OUTPUT:\n" << analyzeResults[0];


    std::vector<std::string> optimizatons = {"100", "010", "001", "110", "101","011", 
                                             "111", "200", "020", "002", "202", "022", "222"};

    std::vector<std::vector<std::string>> results;

    for(auto optimzation:optimizatons){
        logger.log(TransformerToolTest.transform(optimzation), optimzation);
    }
}




