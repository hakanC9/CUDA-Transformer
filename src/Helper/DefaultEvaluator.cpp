#include "Helper/DefaultEvaluator.h"
#include "llvm/Support/raw_ostream.h"
#include <string>



DefaultEvaluator::DefaultEvaluator(const std::string& targetOutput):targetOutput(targetOutput)
{};

double DefaultEvaluator::getAccuracy(){
    llvm::errs() << "Please implement the function!";
    exit(-1);
}

double DefaultEvaluator::getAccuracy(std::string output){
    return output == targetOutput ? 1.0 : 0.0;
};