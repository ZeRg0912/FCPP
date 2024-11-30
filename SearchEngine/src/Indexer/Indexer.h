#ifndef INDEXER_H
#define INDEXER_H

#include <string>
#include <vector>

class Indexer {
public:
    std::vector<std::pair<std::string, int>> index(const std::string& content);

private:
    std::string removeHtmlTags(const std::string& content);
    std::wstring cleanUtf8String(const std::wstring& input);
};

#endif // INDEXER_H
