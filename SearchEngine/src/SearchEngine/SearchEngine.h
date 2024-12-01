#pragma once
#include "../Utils/Logger.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>

#include <thread>
#include <iostream>

class SearchEngine {
public:
    SearchEngine(int port);
    ~SearchEngine();

    void run();
    void stop();

private:
    int port;
    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::acceptor acceptor;
    std::thread ioThread;

    void handleRequest(boost::asio::ip::tcp::socket socket);
};