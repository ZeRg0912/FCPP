#include "SearchEngine.h"

std::string URLDecode(const std::string& encoded) {
    std::string res;
    size_t i = 0;

    while (i < encoded.size()) {
        if (encoded[i] == '%') {
            if (i + 2 < encoded.size() && isxdigit(encoded[i + 1]) && isxdigit(encoded[i + 2])) {
                int hex = 0;
                std::istringstream(encoded.substr(i + 1, 2)) >> std::hex >> hex;
                res += static_cast<char>(hex);
                i += 3;
            } else {
                res += '%';
                i++;
            }
        } else if (encoded[i] == '+') {
            res += ' ';
            i++;
        } else {
            res += encoded[i];
            i++;
        }
    }

    return res;
}

SearchEngine::SearchEngine(Database& db, int port)
    : db(db), port(port), acceptor(ioContext, tcp::endpoint(tcp::v4(), port)),
    deadline_(ioContext), socket_(ioContext) {}

void SearchEngine::run() {
    try {
        //Logger::log("Search engine running on port " + std::to_string(port));
        doAccept();
        ioContext.run();
    }
    catch (const std::exception& e) {
        Logger::logError("Error in run method: " + std::string(e.what()));
    }
}

void SearchEngine::doAccept() {
    if (!acceptor.is_open()) {
        //Logger::log("Acceptor is closed. Stopping doAccept.");
        return;
    }

    acceptor.async_accept(socket_, [this](beast::error_code ec) {
        try {
            if (!ec) {
                //Logger::log("Accepted connection.");
                readRequest();
                checkDeadline();
            }
            else {
                if (ec == net::error::operation_aborted) {
                    Logger::log("Accept operation was aborted (likely due to shutdown).");
                }
                else {
                    //Logger::logInfo("No new connections. Error accepting connection: " + ec.message());
                }
            }
            doAccept();
        }
        catch (const std::exception& e) {
            Logger::logError("Exception in accept handler: " + std::string(e.what()));
        }
        });
}

void SearchEngine::stop() {
    try {
        if (!acceptor.is_open()) {
            Logger::log("Acceptor is already closed.");
            return;
        }

        Logger::log("Stopping Search Engine...");
        beast::error_code ec;

        acceptor.close(ec);
        if (ec) {
            Logger::logError("Error closing acceptor: " + ec.message());
        }

        ioContext.stop();
    }
    catch (const std::exception& e) {
        Logger::logError("Error stopping search engine: " + std::string(e.what()));
    }
}

void SearchEngine::checkDeadline() {
    auto self = shared_from_this();

    deadline_.expires_after(std::chrono::seconds(5));
    //Logger::log("Deadline set for 5 seconds.");

    deadline_.async_wait([self](beast::error_code ec) {
        try {
            if (!ec && self->socket_.is_open()) {
                Logger::logError("Request timed out. Closing socket...");
                beast::error_code close_ec;
                self->socket_.close(close_ec);
            }
            else {
                //Logger::log("Deadline timer cancelled.");
            }
        }
        catch (const std::exception& e) {
            Logger::logError("Exception in checkDeadline: " + std::string(e.what()));
        }
        });
}

void SearchEngine::writeResponse() {
    auto self = shared_from_this();

    response_.content_length(response_.body().size());

    http::async_write(socket_, response_, [self](beast::error_code ec, std::size_t) {
        try {
            if (!ec) {
                //Logger::log("Response sent successfully.");
                self->readRequest();
            }
            else {
                Logger::logError("Error sending response: " + ec.message());
                self->socket_.close();
            }
            self->deadline_.cancel();
        }
        catch (const std::exception& e) {
            Logger::logError("Exception in writeResponse: " + std::string(e.what()));
        }
        });
}

void SearchEngine::readRequest() {
    auto self = shared_from_this();

    request_ = {};

    http::async_read(socket_, buffer_, request_, [self](beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (!ec) {
            try {
                //Logger::log("Request read successfully.");
                self->processRequest();
            }
            catch (const std::exception& e) {
                Logger::logError("Error processing request: " + std::string(e.what()));
            }
        }
        else {
            Logger::logError("Error reading request: " + ec.message());
        }
        });
}

void SearchEngine::processRequest() {
    try {
        //Logger::log("Processing HTTP request...");
        //Logger::log("Request method: " + std::to_string(static_cast<int>(request_.method())));
        //Logger::log("Request target: " + std::string(request_.target()));

        std::string htmlContent;
        std::string query;
        std::string resultsHtml;

        // Загружаем HTML-шаблон
        std::ifstream file("search_form.html");
        if (!file.is_open()) {
            Logger::logError("Failed to open search_form.html");
            response_.result(http::status::internal_server_error);
            response_.body() = "<html><body><h1>500 Internal Server Error</h1></body></html>";
            writeResponse();
            return;
        }
        htmlContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        if (request_.method() == http::verb::post) {
            //Logger::log("POST request received. Processing search query...");
            //Logger::log("Request body: " + request_.body());

            // Извлечение параметра query
            std::string body = request_.body();
            auto pos = body.find("query=");
            if (pos != std::string::npos) {
                query = body.substr(pos + 6);
                query = URLDecode(query); // Декодируем URL-символы
                //Logger::log("Extracted query: " + query);

                try {
                    // Конвертируем string в wstring
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::wstring wideQuery = converter.from_bytes(query);

                    // Приводим к нижнему регистру
                    std::locale loc("ru_RU.UTF-8");
                    for (auto& ch : wideQuery) {
                        ch = std::tolower(ch, loc);
                    }

                    // Конвертируем обратно в string
                    query = converter.to_bytes(wideQuery);
                }
                catch (const std::exception& e) {
                    Logger::logError("Error converting query to lowercase: " + std::string(e.what()));
                }
            }

            // Разбор запроса
            std::vector<std::string> words;
            std::istringstream stream(query);
            for (std::string word; stream >> word;) {
                word.erase(remove_if(word.begin(), word.end(), ispunct), word.end());
                try {
                    word = boost::locale::conv::from_utf(word, "Windows-1251");
                }
                catch (const std::exception& e) {
                    Logger::logError("Error converting word to Windows-1251: " + std::string(e.what()));
                }
                if (!word.empty()) {
                    words.push_back(word);
                }
            }

            auto results = db.getRankedDocuments(words);

            // Формируем HTML для результатов
            if (results.empty()) {
                resultsHtml = "<li>No results found.</li>";
            }
            else {
                for (const auto& [url, relevance] : results) {
                    resultsHtml += "<li><a href=\"" + url + "\">" + url + "</a> - Relevance: " + std::to_string(relevance) + "</li>";
                }
            }
        }

        size_t posQuery = htmlContent.find("{{query}}");
        if (posQuery != std::string::npos) {
            htmlContent.replace(posQuery, 9, query);
        }

        size_t posResults = htmlContent.find("{{results}}");
        if (posResults != std::string::npos) {
            htmlContent.replace(posResults, 11, resultsHtml);
        }

        response_.result(http::status::ok);
        response_.set(http::field::content_type, "text/html");
        response_.body() = htmlContent;

        writeResponse();
    }
    catch (const std::exception& e) {
        Logger::logError("Error in processRequest: " + std::string(e.what()));
        response_.result(http::status::internal_server_error);
        response_.body() = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        writeResponse();
    }
}
