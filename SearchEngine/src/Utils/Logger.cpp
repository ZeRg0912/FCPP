#include "Logger.h"
#include <windows.h>

std::mutex Logger::logMutex;

std::string Logger::UTF8_to_CP1251(const std::string& utf8)
{
    if (!utf8.empty())
    {
        int wchlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
        if (wchlen > 0 && wchlen != 0xFFFD)
        {
            std::vector<wchar_t> wbuf(wchlen);
            MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), &wbuf[0], wchlen);
            std::vector<char> buf(wchlen);
            WideCharToMultiByte(1251, 0, &wbuf[0], wchlen, &buf[0], wchlen, 0, 0);

            return std::string(&buf[0], wchlen);
        }
    }
    return std::string();
}

// Логирование
void Logger::logInfo(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::string message1251 = UTF8_to_CP1251(message);
    std::cout << "\033[0;32m" << "[INFO] " << message1251 << "\033[0m" << std::endl;
}

void Logger::logError(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::string message1251 = UTF8_to_CP1251(message);
    std::cerr << "\033[0;31m" << "[ERROR] " << message1251 << "\033[0m" << std::endl;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::string message1251 = UTF8_to_CP1251(message);
    std::cout << message1251 << std::endl;
}