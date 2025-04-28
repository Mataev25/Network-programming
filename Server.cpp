#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;
using asio::ip::tcp;

// Функция обработки соединения
void handle_connection(tcp::socket& socket) {
    try {
        asio::streambuf buf;
        size_t bytes_read = asio::read_until(socket, buf, '\n');

        std::string message(
            asio::buffers_begin(buf.data()),
            asio::buffers_begin(buf.data()) + bytes_read - 1
        );
        std::cout << "Received: " << message << "\n";

        message += "\n";
        asio::write(socket, asio::buffer(message));

    }
    catch (const std::exception& e) {
        std::cerr << "Connection error: " << e.what() << "\n";
    }

    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);
    if (ec) 
        std::cerr << "Shutdown warning: " << ec.message() << "\n";

    socket.close(ec);

    if (ec) 
        std::cerr << "Close warning: " << ec.message() << "\n";
}

int main() {
    setlocale(LC_ALL, "rus");
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8888));
        std::cout << "Single-threaded echo server started on port 8888\n";

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            std::cout << "Client connected: "
                << socket.remote_endpoint().address().to_string() << "\n";

            handle_connection(socket);
            std::cout << "Connection closed\n\n";
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal server error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}