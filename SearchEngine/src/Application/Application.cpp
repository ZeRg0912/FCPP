#include "Application.h"
#include <iostream>

Application::Application(const std::string& configFile)
    : config(configFile),
    db(
        config.get("database.db_host"),
        config.get("database.db_port"),
        config.get("database.db_name"),
        config.get("database.db_user"),
        config.get("database.db_password")
    ),
    spider(
        db,
        config.get("spider.start_url"),
        std::stoi(config.get("spider.recursion_depth"))
    ),
    searchEngine(std::stoi(config.get("server.server_port"))) {
    config.validateConfig();
    Logger::log("Application initialized with config file: " + configFile);
}

void Application::run() {
    try {
        std::cout << "Application started." << std::endl;

        spiderThread = std::thread(&Application::startSpider, this);
        std::thread serverThread(&SearchEngine::run, &searchEngine);

        if (spiderThread.joinable()) {
            spiderThread.join();
        }

        runInteractiveSearch();

        searchEngine.stop();

        if (serverThread.joinable()) {
            serverThread.join();
        }

        Logger::log("Application finished.");
    }
    catch (const std::exception& e) {
        Logger::logError("Error running application: " + std::string(e.what()));
    }
}


void Application::startSpider() {
    try {
        Logger::log("Starting Spider...");
        spider.run();
    }
    catch (const std::exception& e) {
        Logger::log("Error in Spider: " + std::string(e.what()));
    }
}

void Application::runInteractiveSearch() {
    while (true) {
        std::cout << "Enter your search query (or 0 to exit): ";
        std::string query;
        std::getline(std::cin, query);

        if (query == "0") break;

        std::istringstream stream(query);
        std::vector<std::string> words;
        std::string word;
        while (stream >> word) {
            words.push_back(word);
        }

        // Передаём слова в функцию поиска
        auto results = db.getRankedDocuments(words);

        if (results.empty()) {
            Logger::logError("No results found for your query.");
        }
        else {
            Logger::logInfo("Search results:");
            for (const auto& [url, relevance] : results) {
                Logger::log("URL: " + url + " | Relevance: " + std::to_string(relevance));
            }
        }
    }
}
