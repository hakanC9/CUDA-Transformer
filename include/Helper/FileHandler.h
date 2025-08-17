#pragma once

#include <string>
class FileHandler{

public:

    static void copyTransformedToOriginal(const std::string& optimizationString, const std::string& path);

    static void saveOriginal(const std::string& path);

    static void revertToOriginal(const std::string& path);
};