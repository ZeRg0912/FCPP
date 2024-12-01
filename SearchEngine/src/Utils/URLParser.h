#pragma once
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>

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