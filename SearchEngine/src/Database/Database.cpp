#include "Database.h"

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
        #ifdef FULL_PROJECT_MODE
        Logger::log("Database initialized successfully.");
        #endif
    }
    catch (const std::exception& e) {
        Logger::logError("Failed to initialize database: " + std::string(e.what()));
        throw;
    }
}

int Database::insertDocument(const std::string& url, const std::string& content) {
    std::string html = content;
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
                "UPDATE documents SET content = " + txn.quote(html) +
                " WHERE id = " + txn.quote(documentId) + ";"
            );
            txn.commit();
            return documentId;
        }

        // Вставка нового документа
        result = txn.exec(
            "INSERT INTO documents (url, content) "
            "VALUES (" + txn.quote(url) + ", " + txn.quote(html) + ") "
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

std::vector<std::pair<std::string, int>> Database::getRankedDocuments(const std::vector<std::string>& words) {
    std::vector<std::pair<std::string, int>> results;

    try {
        pqxx::connection connection(connectionString);
        pqxx::work txn(connection);

        // Формируем запрос
        std::ostringstream query;
        query << "SELECT d.url, SUM(w.frequency) AS total_relevance "
            << "FROM documents d "
            << "JOIN words w ON d.id = w.document_id "
            << "WHERE w.word IN (";

        for (size_t i = 0; i < words.size(); ++i) {
            if (i > 0) query << ", ";

            // Конвертируем слово в UTF-8
            std::string utf8Word;
            try {
                utf8Word = boost::locale::conv::to_utf<char>(words[i], "Windows-1251");
            }
            catch (const std::exception& e) {
                Logger::logError("Error converting word to UTF-8: " + std::string(e.what()));
                utf8Word = words[i];
            }

            query << txn.quote(utf8Word);
        }

        query << ") GROUP BY d.url "
            << "ORDER BY total_relevance DESC "
            << "LIMIT 10;";

        // Выполняем запрос
        pqxx::result result = txn.exec(query.str());

        // Обрабатываем результаты
        for (const auto& row : result) {
            std::string url = row["url"].as<std::string>();
            int relevance = row["total_relevance"].as<int>();
            results.emplace_back(url, relevance);
        }
    }
    catch (const std::exception& e) {
        Logger::logError("Error fetching ranked documents: " + std::string(e.what()));
    }

    return results;
}
