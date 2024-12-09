#include "URLParser.h"

ParsedURL URLParser::parse(const std::string& url) {
    const std::regex urlRegex(R"(^(https?)://([^/]+)(/?.*)$)");
    std::smatch match;

    if (!std::regex_match(url, match, urlRegex)) {
        throw std::invalid_argument("Invalid URL format: " + url);
    }

    ProtocolType protocol = (match[1] == "https") ? ProtocolType::HTTPS : ProtocolType::HTTP;
    std::string hostName = match[2];
    std::string query = match[3].str().empty() ? "/" : match[3].str();

    return { protocol, hostName, query };
}

std::vector<std::string> URLParser::extractLinks(const std::string& content, const std::string& baseUrl) {
    std::vector<std::string> links;
    std::regex hrefRegex(R"(<a\s+[^>]*href=["']([^"']+)["'])", std::regex::icase);
    auto begin = std::sregex_iterator(content.begin(), content.end(), hrefRegex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        std::string link = (*it)[1].str();

        // Удаляем фрагмент ссылки (начинается с #)
        size_t hashPos = link.find('#');
        if (hashPos != std::string::npos) {
            link = link.substr(0, hashPos);
        }

        if (link.empty()) {
            continue;
        }

        if (link.find("http://") != 0 && link.find("https://") != 0) {
            if (link.front() == '/') {
                ParsedURL baseParsed = parse(baseUrl);
                link = (baseParsed.protocol == ProtocolType::HTTPS ? "https://" : "http://") + baseParsed.hostName + link;
            }
            else {
                link = baseUrl + "/" + link;
            }
        }

        if (!link.empty() && link.back() == '/') {
            link.pop_back();
        }

        links.push_back(link);
    }
    return links;
}