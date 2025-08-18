#pragma once

#include <map>
#include <string>
#include <vector>


class ConfigParser{
        
public:

    std::map<std::string, std::string> readConfig(); 

    std::string parseOptions(std::string line);

    std::string  parseIncludes(std::string line);

    std::vector<std::string> parseWhitespace(std::string line);

    std::vector<std::string> extractOptimizations(const std::string& optimization_indices, const std::string& optimizationString);
};