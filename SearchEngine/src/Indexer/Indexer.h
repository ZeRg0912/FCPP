#pragma once
#include <string>
#include <vector>

class Indexer {
public:
    std::vector<std::pair<std::string, int>> index(const std::string& content);

private:
    std::string removeHtmlTags(const std::string& content);
};