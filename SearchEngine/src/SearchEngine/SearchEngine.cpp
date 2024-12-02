#include "SearchEngine.h"

SearchEngine::SearchEngine(Database& db, int port)
    : db(db), port(port), acceptor(ioContext, tcp::endpoint(tcp::v4(), port)),
    deadline_(ioContext), socket_(ioContext) {}

void SearchEngine::run() {
    try {
        Logger::log("Search engine running on port " + std::to_string(port));
        doAccept(); // �������� ������� �����������
        ioContext.run(); // ������������ ��� ����������� ������ � ����� ������
    }
    catch (const std::exception& e) {
        Logger::logError("Error in run method: " + std::string(e.what()));
    }
}

void SearchEngine::doAccept() {
    if (!acceptor.is_open()) {
        Logger::log("Acceptor is closed. Stopping doAccept.");
        return; // ���������� ������� ������� ����������, ���� acceptor ������
    }

    acceptor.async_accept(socket_, [this](beast::error_code ec) {
        try {
            if (!ec) {
                Logger::log("Accepted connection.");
                readRequest(); // ��������� �������
                checkDeadline(); // �������� �������
            }
            else {
                // ���� ������ �������� ������� (��������, ���������� �� ���� �����������), ������� � ����
                if (ec == net::error::operation_aborted) {
                    Logger::log("Accept operation was aborted (likely due to shutdown).");
                }
                else {
                    // �������� ��� �������������� ������, ���� ��� ������ ���������� ������ �����������
                    Logger::logInfo("No new connections. Error accepting connection: " + ec.message());
                }
            }
            doAccept(); // ��������� ����� ��� �������� ����� ����������
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

        // ��������� acceptor
        acceptor.close(ec);
        if (ec) {
            Logger::logError("Error closing acceptor: " + ec.message());
        }

        // ��������� io_context
        ioContext.stop();
    }
    catch (const std::exception& e) {
        Logger::logError("Error stopping search engine: " + std::string(e.what()));
    }
}


void SearchEngine::checkDeadline() {
    auto self = shared_from_this();

    deadline_.expires_after(std::chrono::seconds(5));
    Logger::log("Deadline set for 5 seconds.");

    deadline_.async_wait([self](beast::error_code ec) {
        try {
            if (!ec && self->socket_.is_open()) {
                Logger::logError("Request timed out. Closing socket...");
                beast::error_code close_ec;
                self->socket_.close(close_ec);
            }
            else {
                Logger::log("Deadline timer cancelled.");
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
                Logger::log("Response sent successfully.");
                self->readRequest(); // ��������� � ���������� ������� �� ��� �� ������
            }
            else {
                Logger::logError("Error sending response: " + ec.message());
                self->socket_.close(); // ��������� ����� ��� ������
            }
            self->deadline_.cancel(); // ��������� �������
        }
        catch (const std::exception& e) {
            Logger::logError("Exception in writeResponse: " + std::string(e.what()));
        }
        });
}

void SearchEngine::readRequest() {
    auto self = shared_from_this();

    http::async_read(socket_, buffer_, request_, [self](beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (!ec) {
            try {
                Logger::log("Request read successfully.");
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
        Logger::log("Processing HTTP request...");
        Logger::log("Request method: " + std::to_string(static_cast<int>(request_.method())));
        Logger::log("Request target: " + std::string(request_.target()));

        // ��������� ������� �� favicon.ico
        if (request_.method() == http::verb::get && request_.target() == "/favicon.ico") {
            Logger::log("Favicon request received.");
            response_.result(http::status::ok);
            response_.body() = ""; // ������ ����� �� ������ favicon.ico
            writeResponse();
            return; // ��������� ���������, ���������� ������ �����
        }

        // ��������� ������� �� ������� �������� (GET /)
        if (request_.method() == http::verb::get && request_.target() == "/") {
            std::ifstream file("search_form.html");
            if (!file.is_open()) {
                Logger::logError("Failed to open search_form.html");
                response_.result(http::status::internal_server_error);
                response_.body() = "<html><body><h1>500 Internal Server Error</h1></body></html>";
            }
            else {
                std::string htmlContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                response_.result(http::status::ok);
                response_.set(http::field::content_type, "text/html");
                response_.body() = htmlContent;
            }
        }
        else if (request_.method() == http::verb::post) {
            // ��������� POST-�������
            Logger::log("Processing POST request...");
            Logger::log("Request body: " + request_.body());

            // ���������� ��������� query
            std::string body = request_.body();
            std::string query;
            auto pos = body.find("query=");
            if (pos != std::string::npos) {
                query = body.substr(pos + 6);
                Logger::log("Extracted query: " + query);
            }
            else {
                Logger::logError("Query parameter not found in request body.");
                response_.result(http::status::bad_request);
                response_.body() = "<html><body><h1>400 Bad Request</h1><p>Missing query parameter.</p></body></html>";
                writeResponse();
                return;
            }

            // ��������� �������
            std::vector<std::string> words;
            std::istringstream stream(query);
            for (std::string word; stream >> word;) {
                word.erase(remove_if(word.begin(), word.end(), ispunct), word.end());
                words.push_back(word);
            }

            Logger::log("Parsed words: " + std::to_string(words.size()));

            auto results = db.getRankedDocuments(words);

            std::ostringstream responseStream;
            responseStream << "<html><body><h1>Search Results</h1><ul>";

            for (const auto& [url, relevance] : results) {
                responseStream << "<li><a href=\"" << url << "\">" << url << "</a> - Relevance: " << relevance << "</li>";
            }

            responseStream << "</ul></body></html>";
            response_.result(http::status::ok);
            response_.set(http::field::content_type, "text/html");
            response_.body() = responseStream.str();
        }
        else {
            response_.result(http::status::method_not_allowed);
            response_.body() = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        }

        writeResponse();
    }
    catch (const std::exception& e) {
        Logger::logError("Error in processRequest: " + std::string(e.what()));
        response_.result(http::status::internal_server_error);
        response_.body() = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        writeResponse();
    }
}
