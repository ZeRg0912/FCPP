#pragma once
#include <boost/regex.hpp>
#include <boost/locale.hpp>

#include "../Utils/Logger.h"

#include <iostream>
#include <regex>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <codecvt>
#include <string>
#include <vector>

class Indexer {
public:
    std::vector<std::pair<std::string, int>> index(const std::string& content);

private:
    std::string removeHtmlTags(const std::string& content);
};