#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include "URLParser.h"
#include <string>

class HTTPUtils {
public:
    static std::string fetchPage(const ParsedURL& url);
};

#endif // HTTPUTILS_H
