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

    std::cout << "\nANALYZE OUTPUT:\n";

    for(auto a: analyzeResults){
        if(a.empty()){
            std::cout << "-1\n";
        }
        else{
            std::cout << a << "\n";
        }
    }
    
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
    
    */

    std::string opt = "1111111111111111111111111111111111111111111100000000000000000001111111111111111111111111111111111111111111111111111111111111111100000111111111111111111111111111111111111111111111111111111111111111111111";


    logger.log(TransformerToolTest.transform(opt), opt);

    // std::vector<std::string> optimizatons = {"100", "010", "001", "110", "101","011", 
    //                                          "111", "200", "020", "002", "202", "022", "222"};

    // std::vector<std::vector<std::string>> results;

    // for(auto optimzation:optimizatons){
    //     logger.log(TransformerToolTest.transform(optimzation), optimzation);
    // }


    // std::vector<std::string> outputs = TransformerToolTest.transform("100");

    // std::cout <<"TRANSFORMATION RESULTS";
    // for (auto a  : outputs) {
    //     std::cout << "\n" + a + "\n";
    // }
}




