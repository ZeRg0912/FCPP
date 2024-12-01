#pragma once
#include "../Database/Database.h"
#include "../Utils/URLParser.h"
#include "../Utils/HttpUtils.h"
#include "../Utils/Logger.h"
#include "../Indexer/Indexer.h"

#include <thread>
#include <queue>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <condition_variable>

struct Task {
    std::string url;
    int depth;
};

class Spider {
public:
    Spider(Database& db, const std::string& startUrl, int maxDepth);
    void run();

private:
    Database& db;
    std::string startUrl;
    int maxDepth;
    int numThreads; 
    int activeWorkers = 0;
    std::vector<std::thread> threads;
    std::queue<Task> taskQueue;
    std::unordered_set<std::string> visitedUrls;
    std::mutex queueMutex;
    std::mutex visitedMutex;
    std::condition_variable taskCondition;
    bool stopWorkers;

    Indexer indexer;

    void worker();
    void addLinksToQueue(const std::vector<std::string>& links, int depth);
};