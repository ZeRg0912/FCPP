#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

class Config {
public:
    explicit Config(const std::string& configFilePath);
    void validateConfig() const;
    std::string get(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> settings;
    void parseConfigFile(const std::string& configFilePath);
};