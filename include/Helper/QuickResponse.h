#pragma once

#include <iostream>
#include <vector>
#include <string>

class QuickResponse{
public:

    static std::vector<std::string> quickResponse(const std::exception& e, const std::string& optimizationString, const std::string& errorCode);
    static std::vector<std::string> quickResponse(const std::string& errorMsg, const std::string& optimizationString, const std::string& errorCode);

private:
    static std::vector<std::string> quickResponse(const std::string& msg,
                                                  const std::string& optimizationString,
                                                  const std::string& errorCode,
                                                  const std::string& prefix);
};