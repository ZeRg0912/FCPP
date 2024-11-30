#include "SearchEngine.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

SearchEngine::SearchEngine(int port)
    : port(port), acceptor(ioContext, tcp::endpoint(tcp::v4(), port)) {
}

SearchEngine::~SearchEngine() {
    stop();
    if (ioThread.joinable()) {
        ioThread.join();
    }
}

void SearchEngine::run() {
    Logger::log("Search engine running on port " + std::to_string(port) + "...");

    ioThread = std::thread([this]() { ioContext.run(); });

    acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            handleRequest(std::move(socket));
        }
        if (acceptor.is_open()) {
            run();
        }
        });
}

void SearchEngine::stop() {
    if (!acceptor.is_open()) {
        return;
    }
    Logger::log("Stopping Search Engine...");
    ioContext.stop();
    acceptor.close();
}

void SearchEngine::handleRequest(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        std::string responseBody = "Search results for: " + req.body();

        http::response<http::string_body> res{
            http::status::ok, req.version() };
        res.set(http::field::server, "Boost.Beast");
        res.set(http::field::content_type, "text/plain");
        res.body() = responseBody;
        res.prepare_payload();

        http::write(socket, res);
        socket.shutdown(tcp::socket::shutdown_send);
    }
    catch (const std::exception& e) {
        Logger::logError("Error handling request: " + std::string(e.what()));
    }
}