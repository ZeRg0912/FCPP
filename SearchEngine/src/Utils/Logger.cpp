#include "Logger.h"

std::mutex Logger::logMutex;

void Logger::logInfo(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << "\033[0;32m" << "[INFO] " << message << "\033[0m" << std::endl;
}

void Logger::logError(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cerr << "\033[0;31m" << "[ERROR] " << message << "\033[0m" << std::endl;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << message << std::endl;
}
