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

        // Извлечение задачи из очереди
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCondition.wait(lock, [this]() {
                return !taskQueue.empty() || stopWorkers;
                });

            // Проверяем завершение работы
            if (stopWorkers && taskQueue.empty()) {
                break;
            }

            // Извлекаем задачу из очереди
            if (!taskQueue.empty()) {
                task = taskQueue.front();
                taskQueue.pop();
                ++activeWorkers; // Увеличиваем количество активных потоков
            }
            else {
                continue;
            }
        }

        // Проверяем глубину рекурсии
        if (task.depth >= maxDepth) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                --activeWorkers; // Поток завершил задачу
                if (taskQueue.empty() && activeWorkers == 0) {
                    stopWorkers = true; // Завершаем работу, если задач больше нет
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
                    --activeWorkers; // Поток завершил задачу
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
                Logger::logError("Failed to fetch content: " + task.url);
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    --activeWorkers; // Поток завершил задачу
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

            // Извлечение ссылок и добавление новых задач
            if (task.depth < maxDepth - 1) {
                auto links = URLParser::extractLinks(content, task.url);

                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    for (const auto& link : links) {
                        if (visitedUrls.find(link) == visitedUrls.end()) {
                            taskQueue.push({ link, task.depth + 1 });
                        }
                    }

                    // Логируем общее количество ссылок, если добавлены новые задачи
                    Logger::logInfo("Total links in queue: " + std::to_string(taskQueue.size()));
                }

                // Уведомляем другие потоки о новых задачах
                taskCondition.notify_all();
            }
        }
        catch (const std::exception& e) {
            Logger::logError("Error processing URL: " + task.url + " - " + std::string(e.what()));
        }

        // Завершаем текущую задачу
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            --activeWorkers; // Уменьшаем количество активных потоков
            if (taskQueue.empty() && activeWorkers == 0) {
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
    taskCondition.notify_all();
}
