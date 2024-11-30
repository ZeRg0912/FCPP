#include "Database.h"
#include <iostream>

Database::Database(const std::string& host, const std::string& port, const std::string& name,
    const std::string& user, const std::string& password)
    : connectionString("host=" + host + " port=" + port + " dbname=" + name +
        " user=" + user + " password=" + password) {
    initializeDatabase();
}

void Database::initializeDatabase() {
    try {
        pqxx::connection connection(connectionString);
        pqxx::work txn(connection);

        txn.exec(
            "CREATE TABLE IF NOT EXISTS documents ("
            "id SERIAL PRIMARY KEY, "
            "url TEXT NOT NULL UNIQUE, "
            "content TEXT NOT NULL);"
        );

        txn.exec(
            "CREATE TABLE IF NOT EXISTS words ("
            "id SERIAL PRIMARY KEY, "
            "document_id INT NOT NULL REFERENCES documents(id) ON DELETE CASCADE, "
            "word TEXT NOT NULL, "
            "frequency INT NOT NULL);"
        );

        txn.commit();
        std::cout << "Database initialized successfully.\n";
    }
    catch (const std::exception& e) {
        Logger::logError("Failed to initialize database: " + std::string(e.what()));
        throw;
    }
}

int Database::insertDocument(const std::string& url, const std::string& content) {
    std::string sanitizedContent = sanitizeContent(content);
    try {
        pqxx::connection connection(connectionString);
        pqxx::work txn(connection);

        // Проверка существующего документа
        pqxx::result result = txn.exec(
            "SELECT id FROM documents WHERE url = " + txn.quote(url) + ";"
        );

        if (!result.empty()) {
            int documentId = result[0][0].as<int>();
            txn.exec(
                "UPDATE documents SET content = " + txn.quote(sanitizedContent) +
                " WHERE id = " + txn.quote(documentId) + ";"
            );
            txn.commit();
            return documentId;
        }

        // Вставка нового документа
        result = txn.exec(
            "INSERT INTO documents (url, content) "
            "VALUES (" + txn.quote(url) + ", " + txn.quote(sanitizedContent) + ") "
            "RETURNING id;"
        );

        txn.commit();
        return result[0][0].as<int>();
    }
    catch (const std::exception& e) {
        Logger::logError("Error inserting or updating document: " + std::string(e.what()));
        throw;
    }
}

void Database::insertWords(const std::vector<std::pair<std::string, int>>& wordFrequency, int documentId) {
    try {
        pqxx::connection connection(connectionString);
        pqxx::work txn(connection);

        // Удаляем старые слова для данного документа
        txn.exec(
            "DELETE FROM words WHERE document_id = " + txn.quote(documentId) + ";"
        );

        // Вставляем новые слова
        for (const auto& [word, freq] : wordFrequency) {
            txn.exec(
                "INSERT INTO words (document_id, word, frequency) "
                "VALUES (" + txn.quote(documentId) + ", " + txn.quote(word) + ", " + txn.quote(freq) + ");"
            );
        }

        txn.commit();
        Logger::logInfo("Words updated successfully for document ID " + std::to_string(documentId));
    }
    catch (const std::exception& e) {
        Logger::logError("Error updating words: " + std::string(e.what()));
        throw;
    }
}

std::string Database::sanitizeContent(const std::string& content) {
    std::string sanitized;
    for (unsigned char c : content) {
        if (c >= 0x20 || c == '\n' || c == '\t') {
            sanitized += c;
        }
        else {
            sanitized += ' ';
        }
    }
    return sanitized;
}