#include "Transformer/CUDA_TransformerTool.h"
#include "Helper/Logger.h"

/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;
    Logger logger;

    // 11343434343434566777777
    
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();

    std::string optimization = "11111111111111111111111";
    logger.log(TransformerToolTest.transform(optimization), optimization);
}




