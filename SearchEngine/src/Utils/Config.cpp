#include "Config.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

Config::Config(const std::string& configFilePath) {
    parseConfigFile(configFilePath);
}

void Config::parseConfigFile(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configFilePath);
    }

    std::string line;
    std::string currentSection;

    while (std::getline(file, line)) {
        // Убираем пробелы в начале и конце строки
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Игнорируем пустые строки и комментарии
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

        // Обработка секции
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
        }
        else {
            std::istringstream lineStream(line);
            std::string key, value;

            if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);

                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (!currentSection.empty()) {
                    key = currentSection + "." + key;
                }

                settings[key] = value;
            }
        }
    }
}

std::string Config::get(const std::string& key) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        std::cout << "Accessing config: " << key << " = " << it->second << std::endl;
        return it->second;
    }
    throw std::runtime_error("Config key not found: " + key);
}