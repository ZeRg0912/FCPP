#include "Config.h"

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
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }

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
    #ifdef FULL_PROJECT_MODE
    std::cout << "Accessing config: " << key << " = " << it->second << std::endl;
    #endif
        return it->second;
    }
    throw std::runtime_error("Config key not found: " + key);
}

void Config::validateConfig() const {
    const std::vector<std::string> requiredKeys = {
        "spider.start_url",
        "spider.recursion_depth",
        "server.server_port",
        "database.db_host",
        "database.db_port",
        "database.db_name",
        "database.db_user",
        "database.db_password"
    };

    for (const auto& key : requiredKeys) {
        if (settings.find(key) == settings.end()) {
            throw std::runtime_error("Missing configuration key: " + key);
        }
    }

    int recursionDepth = std::stoi(get("spider.recursion_depth"));
    if (recursionDepth <= 0) {
        throw std::runtime_error("Invalid recursion depth: must be > 0");
    }

    int serverPort = std::stoi(get("server.server_port"));
    if (serverPort <= 1024 || serverPort > 65535) {
        throw std::runtime_error("Invalid server port: must be between 1025 and 65535");
    }
}