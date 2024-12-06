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
    searchEngine(std::make_shared<SearchEngine>(db, std::stoi(config.get("server.server_port")))) {
    config.validateConfig();
#ifdef FULL_PROJECT_MODE
    Logger::log("Application initialized with config file: " + configFile);
#endif
}

void Application::run() {
    try {
        std::cout << "Application started." << std::endl;

        spiderThread = std::thread(&Application::startSpider, this);

        serverThread = std::thread(&SearchEngine::run, searchEngine.get());

        if (spiderThread.joinable()) {
            spiderThread.join();
        }

        Logger::log("Spider completed. Search Engine is running at: http://localhost:" + config.get("server.server_port"));

        Logger::log("Press Enter to stop the server...");
        std::cin.get();

        searchEngine->stop();

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

void Application::ConsoleSearch() {
    try {
        while (true) {
            std::cout << "\033[1;33m"
                << "Enter your search query (or type /exit_search to exit): "
                << "\033[0m";

            std::string query;
            std::getline(std::cin, query);

            if (query == "/exit_search") break;

            // Проверка на пустой запрос
            if (query.empty()) {
                Logger::logError("Query cannot be empty.");
                std::cout << "Query cannot be empty. Please enter a valid search query." << std::endl;
                continue; // Возвращаемся к следующему запросу
            }

            std::istringstream stream(query);
            std::vector<std::string> words;
            std::string word;
            while (stream >> word) {
                words.push_back(word);
            }

            // Получаем результаты из базы данных
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
    catch (const std::exception& e) {
        Logger::logError("Error in loop console search: " + std::string(e.what()));
    }
}


void Application::HTMLSearch() {
    try {
        serverThread = std::thread(&SearchEngine::run, searchEngine.get());
        Logger::log("Starting Search. Access it at: http://localhost:" + config.get("server.server_port"));

        Logger::log("Press Enter to stop the server...");
        std::cin.get();

        searchEngine->stop();

        if (serverThread.joinable()) {
            serverThread.join();
        }
    }
    catch (const std::exception& e) {
        Logger::logError("Error running HTML Search Engine: " + std::string(e.what()));
    }
}