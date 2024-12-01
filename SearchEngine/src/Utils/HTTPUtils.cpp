#include "HTTPUtils.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

std::string HTTPUtils::fetchPage(const ParsedURL& parsedUrl) {
    net::io_context ioc;

    try {
        if (parsedUrl.protocol == ProtocolType::HTTPS) {
            net::ssl::context sslContext(net::ssl::context::tlsv12_client);
            sslContext.set_verify_mode(net::ssl::verify_none);
            sslContext.set_default_verify_paths();

            beast::ssl_stream<beast::tcp_stream> stream(ioc, sslContext);
            tcp::resolver resolver(ioc);
            auto const results = resolver.resolve(parsedUrl.hostName, "443");

            beast::get_lowest_layer(stream).connect(results);
            if (!SSL_set_tlsext_host_name(stream.native_handle(), parsedUrl.hostName.c_str())) {
                throw beast::system_error(net::error::invalid_argument, "Failed to set SNI host name");
            }

            stream.handshake(net::ssl::stream_base::client);
            http::request<http::string_body> req{ http::verb::get, parsedUrl.query, 11 };
            req.set(http::field::host, parsedUrl.hostName);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            http::write(stream, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(stream, buffer, res);

            return res.body();
        }
        else {
            tcp::resolver resolver(ioc);
            beast::tcp_stream stream(ioc);

            auto const results = resolver.resolve(parsedUrl.hostName, "80");
            stream.connect(results);

            http::request<http::string_body> req{ http::verb::get, parsedUrl.query, 11 };
            req.set(http::field::host, parsedUrl.hostName);
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            http::write(stream, req);

            beast::flat_buffer buffer;
            http::response<http::string_body> res;
            http::read(stream, buffer, res);

            return res.body();
        }
    }
    catch (const boost::system::system_error& e) {
        Logger::logError("Failed to connect to host: " + parsedUrl.hostName + ". The server may be down or unreachable.");
        return "";
    }
    catch (const std::exception& e) {
        Logger::logError("Error fetching content from " + parsedUrl.hostName + ": " + e.what());
        return "";
    }
}