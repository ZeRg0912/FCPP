#pragma once
#include "Logger.h"
#include "URLParser.h"

#include <boost/locale.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <regex>

class HTTPUtils {
public:
    static std::string fetchPage(const ParsedURL& url);
};