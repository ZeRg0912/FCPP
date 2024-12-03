#include "Spider.h"

Spider::Spider(Database& database, const std::string& startUrl, int maxDepth)
    : db(database), startUrl(startUrl), maxDepth(maxDepth), stopWorkers(false) {
    numThreads = std::thread::hardware_concurrency();
    #ifdef FULL_PROJECT_MODE
    Logger::log("Number of threads: " + std::to_string(numThreads));
    Logger::log("Spider initialized with max depth: " + std::to_string(maxDepth));
    #endif
}

void Spider::run() {
    try {
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
    catch (const std::exception& e) {
        Logger::logError("Error spider running: " + std::string(e.what()));
    }
}

void Spider::worker() {
    try {
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
                    ++activeWorkers;
                }
                else {
                    continue;
                }
            }

            if (task.depth >= maxDepth) {
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    --activeWorkers;
                    if (taskQueue.empty() && activeWorkers == 0) {
                        stopWorkers = true;
                        taskCondition.notify_all();
                    }
                }
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(visitedMutex);
                if (visitedUrls.count(task.url)) {
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        --activeWorkers;
                        if (taskQueue.empty() && activeWorkers == 0) {
                            stopWorkers = true;
                            taskCondition.notify_all();
                        }
                    }
                    continue;
                }
                visitedUrls.insert(task.url);
            }

            // Обработка URL
            try {
                Logger::log("Processing URL: " + task.url);
                ParsedURL parsedUrl = URLParser::parse(task.url);
                std::string content = HTTPUtils::fetchPage(parsedUrl);

                if (content.empty()) {
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        --activeWorkers;
                        if (taskQueue.empty() && activeWorkers == 0) {
                            stopWorkers = true;
                            taskCondition.notify_all();
                        }
                    }
                    continue;
                }

                int documentId = db.insertDocument(task.url, content);

                auto wordFrequency = indexer.index(content);
                db.insertWords(wordFrequency, documentId);

                if (task.depth < maxDepth - 1) {
                    auto links = URLParser::extractLinks(content, task.url);

                    {
                        addLinksToQueue(links, task.depth + 1);
                        Logger::logInfo("Total links in queue: " + std::to_string(taskQueue.size()));
                    }

                    taskCondition.notify_all();
                }
            }
            catch (const std::exception& e) {
                Logger::logError("Error processing URL: " + task.url + " - " + std::string(e.what()));
            }

            {
                std::lock_guard<std::mutex> lock(queueMutex);
                --activeWorkers;
                if (taskQueue.empty() && activeWorkers == 0) {
                    stopWorkers = true;
                    taskCondition.notify_all();
                }
            }
        }
    }
    catch (const std::exception& e) {
        Logger::logError("Error in spider worker" + std::string(e.what()));
    }
}

void Spider::addLinksToQueue(const std::vector<std::string>& links, int depth) {
    try {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (const auto& link : links) {
            if (visitedUrls.find(link) == visitedUrls.end()) {
                taskQueue.push({ link, depth });
            }
        }
        Logger::logInfo("Added " + std::to_string(links.size()) + " links to queue.");
        taskCondition.notify_all();
    }
    catch (const std::exception& e) {
        Logger::logError("Error on add links to queue" + std::string(e.what()));
    }
}
