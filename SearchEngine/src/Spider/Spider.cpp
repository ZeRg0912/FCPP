#include "Spider.h"

Spider::Spider(Database& database, const std::string& startUrl, int maxDepth)
    : db(database), startUrl(startUrl), maxDepth(maxDepth), stopWorkers(false) {
    numThreads = std::thread::hardware_concurrency();
    Logger::log("Number of threads: " + std::to_string(numThreads));
    Logger::log("Spider initialized with max depth: " + std::to_string(maxDepth));
}

void Spider::run() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push({ startUrl, 0 });
    }

    taskCondition.notify_all();

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(&Spider::worker, this);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    Logger::log("Spider finished. All tasks processed.");
}

void Spider::worker() {
    while (true) {
        Task task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCondition.wait(lock, [this]() {
                return !taskQueue.empty() || stopWorkers;
                });

            if (stopWorkers && taskQueue.empty()) {
                break;
            }

            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
            }
        }

        if (task.depth >= maxDepth) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(visitedMutex);
            if (visitedUrls.count(task.url)) {
                continue;
            }
            visitedUrls.insert(task.url);
        }

        try {
            Logger::log("Processing URL: " + task.url);
            ParsedURL parsedUrl = URLParser::parse(task.url);
            std::string content = HTTPUtils::fetchPage(parsedUrl);

            if (content.empty()) {
                Logger::logError("Failed to fetch content: " + task.url);
                continue;
            }

            int documentId = db.insertDocument(task.url, content);

            auto wordFrequency = indexer.index(content);
            db.insertWords(wordFrequency, documentId);

            if (task.depth < maxDepth - 1) {
                auto links = URLParser::extractLinks(content, task.url);
                addLinksToQueue(links, task.depth + 1);
                taskCondition.notify_all();
            }
        }
        catch (const std::exception& e) {
            Logger::logError("Error processing task: " + std::string(e.what()));
        }

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (taskQueue.empty()) {
                stopWorkers = true;
                taskCondition.notify_all();
            }
        }
    }
}

void Spider::addLinksToQueue(const std::vector<std::string>& links, int depth) {
    std::lock_guard<std::mutex> lock(queueMutex);
    for (const auto& link : links) {
        if (visitedUrls.find(link) == visitedUrls.end()) {
            taskQueue.push({ link, depth });
        }
    }
    Logger::logInfo("Added " + std::to_string(links.size()) + " links to queue.");
}
