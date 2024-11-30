#pragma once
#include <boost/locale.hpp>

#include <string>
#include <mutex>
#include <iostream>

class Logger {
public:
    static void logInfo(const std::string& message);
    static void logError(const std::string& message);
    static void log(const std::string& message);

private:
    static std::mutex logMutex;
    static std::string UTF8_to_CP1251(const std::string& utf8);
};
