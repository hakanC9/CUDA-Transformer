#include "Helper/ConfigParser.h"

#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <sstream>

/**
 * @brief A function to read config.txt file
 * @return std::map<std::string, std::string> with keys as lhs and values as rhs
*/
std::map<std::string, std::string> ConfigParser::readConfig() {
    std::map<std::string, std::string> configMap;
    std::ifstream file("../config.txt");
    std::string line;
    
    while (std::getline(file, line)) {

        if (!line.empty() && line[0] == '/'){
            continue;
        }
        
        if (!line.empty() && line[0] == '='){
            continue;
        }
        
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {

                if(key == "includes"){
                    configMap[key] = parseIncludes(value);
                }
                else{
                    configMap[key] = parseOptions(value);
                }
                
            }
        }
    }
    return configMap;
}


/**
 * @brief A helper function that parses config.txt file
 * @param line   A string with semicolumn seperated values
 * @return std::string whitespace seperated compile options
*/
std::string ConfigParser::parseOptions(std::string line){

    std::stringstream ss(line);

    std::string options;
    std::string temp;

    while(std::getline(ss, temp, ';')){
        options += temp + " ";
    }

    return options.substr(0, options.size()-1);
}


/**
 * @brief A helper function that parses config.txt files includes key
 * @param line   A string with semicolumn seperated values
 * @return std::string all parsed include options with -I prefix
*/
std::string  ConfigParser::parseIncludes(std::string line){

    std::stringstream ss(line);

    std::string includes;
    std::string temp;

    while(std::getline(ss, temp, ';')){
        includes += "-I" + temp + " ";
    }

    return includes.substr(0, includes.length()-1);
}


std::vector<std::string> ConfigParser::parseWhitespace(std::string line){

    std::stringstream ss(line);

    std::vector<std::string> values;
    std::string temp;

    while(std::getline(ss, temp, ' ')){
        values.push_back(temp);
    }

    return values;
}

