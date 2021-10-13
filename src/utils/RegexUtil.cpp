#include "RegexUtil.h"
#include <regex>

std::vector<std::string> cRegexUtil::FindAllMatch(std::string whole_str, std::string regex)
{
    std::smatch matches;
    std::regex_search(whole_str, matches, std::regex(regex));
    std::vector<std::string> sets(0);
    for (auto &i : matches)
    {
        sets.push_back(i);
    }
    return sets;
}