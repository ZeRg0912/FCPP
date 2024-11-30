#ifndef LOGGER_H
#define LOGGER_H

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
};

#endif // LOGGER_H
