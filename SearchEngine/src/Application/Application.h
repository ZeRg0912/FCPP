#pragma once
#include "../Utils/Config.h"
#include "../Utils/Logger.h"
#include "../Database/Database.h"
#include "../Spider/Spider.h"
#include "../SearchEngine/SearchEngine.h"

#include <iostream>
#include <thread>

class Application {
public:
    Application(const std::string& configFile);

    void run();
    void ConsoleSearch();
    void HTMLSearch();

private:
    Config config;
    Database db;
    Spider spider;
    std::shared_ptr<SearchEngine> searchEngine;
    std::thread spiderThread;
    std::thread serverThread;

    void startSpider();
};
