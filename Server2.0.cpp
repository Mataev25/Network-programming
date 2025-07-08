#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <memory>
#include <thread>
#include <vector>

using namespace boost::asio;

struct Session :std::enable_shared_from_this<Session> {
	explicit Session(ip::tcp::socket socket):socket{std::move(socket)}{}
	void read() {
		async_read_until(socket, dynamic_buffer(message), '\r',
			[self = shared_from_this()](boost::system::error_code ec, size_t length) {
				if (ec || self->message.empty()) return;
				boost::algorithm::to_upper(self->message);
				self->write();
			});
	}
	void write() {
		async_write(socket, buffer(message), [self = shared_from_this()](boost::system::error_code ec,
			std::size_t length) {
				if (ec) return;
				self->message.clear();
				boost::system::error_code ignore_ec;
				self->socket.shutdown(ip::tcp::socket::shutdown_both, ignore_ec);
				self->read();
			});
	}
private:
	ip::tcp::socket socket;
	std::string message;
};

void serve(ip::tcp::acceptor& acceptor) {
	acceptor.async_accept([&acceptor](boost::system::error_code ec,
		ip::tcp::socket socket) {
			serve(acceptor);
			if (ec) return;
			auto session = std::make_shared<Session>(std::move(socket));
			session->read();
		});
}

int main() {
	try {
		int thread_pool_size = std::thread::hardware_concurrency();
		io_context io_context{ thread_pool_size };
		ip::tcp::acceptor acceptor{ io_context,ip::tcp::endpoint(ip::tcp::v4(),8888) };
		serve(acceptor);
		std::vector<std::thread> threads;
		for (int i = 0; i < thread_pool_size; i++) {
			threads.emplace_back([&io_context] {
				io_context.run();
				});
		}
		for (auto& thread : threads) {
			thread.join();
		}
	
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
