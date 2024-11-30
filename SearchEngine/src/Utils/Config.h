#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class Config {
public:
    explicit Config(const std::string& configFilePath);
    std::string get(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> settings;
    void parseConfigFile(const std::string& configFilePath);
};

#endif // CONFIG_H
