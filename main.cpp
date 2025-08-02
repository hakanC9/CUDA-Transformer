#include "Transformer/CUDA_TransformerTool.h"
#include <iostream>
#include <string>
#include <vector>
#include "Helper/Logger.h"


/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;
    Logger logger;
    
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();

    std::string opt = "Your Optimization String to Apply";

    logger.log(TransformerToolTest.transform(opt), opt);
}




