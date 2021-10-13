#pragma once
#include <vector>
#include <string>
class cRegexUtil
{
public:
    static std::vector<std::string> FindAllMatch(std::string whole_str, std::string regex);
};