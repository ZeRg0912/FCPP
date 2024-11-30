#ifndef URLPARSER_H
#define URLPARSER_H

#include <string>
#include <vector>

enum class ProtocolType {
    HTTP,
    HTTPS
};

struct ParsedURL {
    ProtocolType protocol;
    std::string hostName;
    std::string query;
};

class URLParser {
public:
    static ParsedURL parse(const std::string& url);
    static std::vector<std::string> extractLinks(const std::string& content, const std::string& baseUrl);
};

#endif // URLPARSER_H
