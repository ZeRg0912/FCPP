#pragma once
#include "URLParser.h"
#include <boost/locale.hpp>
#include <string>

class HTTPUtils {
public:
    static std::string fetchPage(const ParsedURL& url);
};