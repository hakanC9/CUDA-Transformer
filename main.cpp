#include "Transformer/CUDA_TransformerTool.h"

/*
* Example usage of the tool
*/
int main(int argc, char** arv) {

    CUDA_TransformerTool TransformerToolTest;


    std::vector<std::string>results = TransformerToolTest.transform();


    for(auto result : results){
        std::cout << result + "\n";
    }

}
