#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <iostream>
#include <vector>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Function to handle HTTP requests
void handleRequest(http::request<http::string_body>&& req, http::response<http::string_body>& res, int port) {
    if (req.method() == http::verb::get && req.target() == "/") {
        res.result(http::status::ok);
        res.body() = "Response from server running on port " + std::to_string(port);
        res.prepare_payload();
    } else {
        res.result(http::status::not_found);
        res.body() = "Not Found";
        res.prepare_payload();
    }
}

// Function to create an HTTP server on a specific port
void createServer(int port) {
    try {
        net::io_context ioc{1};
        tcp::acceptor acceptor{ioc, tcp::endpoint{tcp::v4(), static_cast<unsigned short>(port)}};
        std::cout << "Server is running on port " << port << std::endl;

        for (;;) {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            http::response<http::string_body> res;
            handleRequest(std::move(req), res, port);

            http::write(socket, res);
            socket.shutdown(tcp::socket::shutdown_send);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error on port " << port << ": " << e.what() << std::endl;
    }
}

int main() {
    const std::vector<int> ports = {8001, 8002, 8003};
    std::vector<std::thread> threads;

    // Start servers on each port in separate threads
    for (int port : ports) {
        threads.emplace_back(createServer, port);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
