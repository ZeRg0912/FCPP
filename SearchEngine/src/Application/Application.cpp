#include "Application.h"

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
    #ifdef FULL_PROJECT_MODE
    Logger::log("Application initialized with config file: " + configFile);
    #endif
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
        std::cout << "\033[1;33m"
            << "Enter your search query (or type /exit_search to exit): "
            << "\033[0m";

        std::string query;
        std::getline(std::cin, query);

        if (query == "/exit_search") break;

        std::istringstream stream(query);
        std::vector<std::string> words;
        std::string word;
        while (stream >> word) {
            words.push_back(word);
        }

        auto results = db.getRankedDocuments(words);

        if (results.empty()) {
            Logger::logError("No results found.");
        }
        else {
            Logger::logInfo("Search results:");
            std::cout << "--------------------------------------------------------------------------------------" << std::endl;
            std::cout << std::left << std::setw(50) << "URL" << std::setw(10) << "Relevance" << std::endl;
            std::cout << "--------------------------------------------------------------------------------------" << std::endl;

            for (const auto& [url, relevance] : results) {
                std::cout << std::left << std::setw(50) << url << std::setw(10) << relevance << std::endl;
            }

            std::cout << "--------------------------------------------------------------------------------------" << std::endl;
        }
    }
}