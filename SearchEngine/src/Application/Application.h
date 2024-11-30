#ifndef APPLICATION_H
#define APPLICATION_H

#include "../Utils/Config.h"
#include "../Utils/Logger.h"
#include "../Database/Database.h"
#include "../Spider/Spider.h"
#include "../SearchEngine/SearchEngine.h"
#include <thread>

class Application {
public:
    Application(const std::string& configFile);
    void run();

private:
    Config config;
    Database db;
    Spider spider;
    SearchEngine searchEngine;
    std::thread spiderThread;

    void startSpider();
};

#endif // APPLICATION_H
