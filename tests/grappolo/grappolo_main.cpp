#include "Transformer/CUDA_TransformerTool.h"
#include <string>
#include <vector>
#include "Helper/Logger.h"

/*
* Example usage of the tool
*/
int main(int argc, char** argv) {


    CUDA_TransformerTool TransformerToolTest;
    Logger logger;
    
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();

    /*
    Analyze Output
    ==============
    343434
    -1
    00000000000000000011111111111111111111222222222222222222266666666666666666666666666666666666666666666888878877877778777777
    22222
    -1
    111166667
    3466666677
    -1
    343434343434343434343434343434343434
    343434343434
    ==============
    */

    std::string opt = "1111111111111111111111111111111111111111111100000000000000000001111111111111111111111111111111111111111111111111111111111111111100000111111111111111111111111111111111111111111111111111111111111111111111";

    logger.log(TransformerToolTest.transform(opt), opt);
}




