#include "Transformer/CUDA_TransformerTool.h"

/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;
    std::vector<std::string> analyzeResults = TransformerToolTest.analyze();
    
    for(auto result:analyzeResults){
        if(result.empty()){
            std::cout << "-1\n";
        }
        else{
            std::cout << result << "\n";
        }
    }

    //TransformerToolTest.transformOnly();

    //TransformerToolTest.transformSingle();
}




