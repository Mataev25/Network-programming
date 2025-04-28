#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <string>

namespace asio = boost::asio;
using asio::ip::tcp;

void sync_echo(const std::string& msg) {
    try {
        asio::io_context io_context;
        tcp::socket sock(io_context);

        tcp::endpoint ep(asio::ip::make_address("127.0.0.1"), 8888);
        sock.connect(ep);

        std::string message = msg + "\n";
        asio::write(sock, asio::buffer(message));

        asio::streambuf buf;
        size_t bytes = asio::read_until(sock, buf, '\n');

        std::string reply(
            asio::buffers_begin(buf.data()),
            asio::buffers_begin(buf.data()) + bytes - 1);

        std::cout << "server echoed our " << msg << ": "
            << (reply == msg ? "OK" : "FAIL") << std::endl;

        sock.shutdown(tcp::socket::shutdown_both);
        boost::system::error_code ec;
        sock.close(ec);
        if (ec) 
            std::cerr << "Close error: " << ec.message() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error with '" << msg << "': " << e.what() << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "rus");
    const char* messages[] = {
        "Илья зашел на сайт",
        "Ксения прошла регистрацию!",
        "Андрею нравится наша публикация!",
        "Мария поделилась ссылкой",
        nullptr
    };

    for (const char** msg = messages; *msg; ++msg) {
        sync_echo(*msg);
        boost::this_thread::sleep_for(boost::chrono::milliseconds(100));
    }

    return 0;
}






