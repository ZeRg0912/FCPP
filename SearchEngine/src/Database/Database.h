#pragma once
#include "../Utils/Logger.h"
#include "../Utils/HTTPUtils.h"

#include <boost/locale.hpp>

#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <stdexcept>
#include <vector>
#include <pqxx/pqxx>

class Database {
public:
    Database(const std::string& host, const std::string& port, const std::string& name,
        const std::string& user, const std::string& password);

    int insertDocument(const std::string& url, const std::string& content);
    void insertWords(const std::vector<std::pair<std::string, int>>& wordFrequency, int documentId);
    std::vector<std::pair<std::string, int>> getRankedDocuments(const std::vector<std::string>& words);

private:
    std::string connectionString;
    void initializeDatabase();
};
