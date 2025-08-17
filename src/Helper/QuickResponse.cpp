#include "Helper/QuickResponse.h"


/**
 * @brief A function to quickly generate a response for exceptions
 *
 * @param e                     Caught exception
 * @param optimizationString    A string of optimization to apply for each target file (example: 11111111)
 * @param errorCode             -1 -> ClangTool -2 -> Timeout -3 -> Build -4 -> Run
 *
 * @return std::vector<string> : 
*/
std::vector<std::string> QuickResponse::quickResponse(const std::exception& e, const std::string& optimizationString, const std::string& errorCode){
    return quickResponse(e.what(), optimizationString, errorCode, "[Exception]");
}


/**
 * @brief A function to quickly generate a response for command errors
 *
 * @param errorMsg              Command error
 * @param optimizationString    A string of optimization to apply for each target file (example: 11111111)
 * @param errorCode             -1 -> ClangTool -2 -> Timeout -3 -> Build -4 -> Run
 *
 * @return std::vector<string> : 
*/
std::vector<std::string> QuickResponse::quickResponse(const std::string& errorMsg, const std::string& optimizationString, const std::string& errorCode){
    return quickResponse(errorMsg, optimizationString, errorCode, "[Error]");
}


/**
 * @brief A function to quickly generate a response for errors
 *
 * @param msg                   Error message 
 * @param optimizationString    A string of optimization to apply for each target file (example: 11111111)
 * @param errorCode             -1 -> ClangTool -2 -> Timeout -3 -> Build -4 -> Run
 * @param prefix                Prefix indicating [Exception] or [Error]
 *
 * @return std::vector<string> : 
*/
std::vector<std::string> QuickResponse::quickResponse(const std::string& msg, 
                                                      const std::string& optimizationString,
                                                      const std::string& errorCode,
                                                      const std::string& prefix){

        std::cerr << prefix << " " << msg << "\n";
        std::cerr << "Could not complete the optimization for:\n";
        std::cerr << optimizationString << "\nReverting back the files\n";

        return {errorCode, errorCode, errorCode, errorCode};                                           
}