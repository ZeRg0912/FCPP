#ifndef DATABASE_H
#define DATABASE_H

#include "../Utils/Logger.h"

#include <string>
#include <vector>
#include <pqxx/pqxx>

class Database {
public:
    Database(const std::string& host, const std::string& port, const std::string& name,
        const std::string& user, const std::string& password);

    int insertDocument(const std::string& url, const std::string& content);
    void insertWords(const std::vector<std::pair<std::string, int>>& wordFrequency, int documentId);

private:
    std::string connectionString;
    void initializeDatabase();
    std::string sanitizeContent(const std::string& content);
};

#endif // DATABASE_H
