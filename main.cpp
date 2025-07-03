#include "Transformer/CUDA_TransformerTool.h"

/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;


    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();
     
    for(auto result : analyzeResults){
        std::cout << result + "\n";
    }

    // std::vector<std::string> transformResults = TransformerToolTest.transform("11111111", "5-3");


    // for(auto result : transformResults){
    //     std::cout << result + "\n";
    // }

}
