#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <sstream>

using ResolveResult = boost::asio::ip::tcp::resolver::results_type;
using Endpoint = boost::asio::ip::tcp::endpoint;

struct Request {
	explicit Request(boost::asio::io_context& io_context, std::string host, int device_id) :
		resolver{ io_context }, socket{ io_context }, host{ std::move(host) }, device_id{device_id} {
		std::stringstream request_stream;
		request_stream << device_message(device_id);
			
		request = request_stream.str();
		resolver.async_resolve(this->host, "8888", [this](boost::system::error_code ec, const ResolveResult& results) {
			resolution_handler(ec, results); 
			});
	}
	std::string device_message(int id) {
		static const std::vector<std::string> device_types = {
			"IOS", "Android","Windows","MacOC"};
		std::stringstream msg;
		msg << "Device ID: " << id << "\r\n"
			<< "Type: " << device_types[id % device_types.size()] << "\r\n"
			<< "TimeStamp: " << time(nullptr) << "\r\n\r\n";
		return msg.str();
	}

	void resolution_handler(boost::system::error_code ec, const ResolveResult& results) {
		if (ec) {
			std::cerr << "Error resolving " << host << ": " << ec << std::endl;
			return;
		}
		boost::asio::async_connect(socket, results, [this](boost::system::error_code ec, const Endpoint& endpoint) {
			connection_handler(ec, endpoint); 
			});
	}
	void connection_handler(boost::system::error_code ec, const Endpoint& endpoint) {
		if (ec) {
			std::cerr << "Error connecting to " << host << ": " << ec.message() << std::endl;
			return;
		}
		boost::asio::async_write(socket, boost::asio::buffer(request), [this](boost::system::error_code ec,
			size_t transferred) {
				write_handler(ec, transferred);
			});
	}
	void write_handler(boost::system::error_code ec, size_t transferred) {
		if (ec) {
			std::cerr << "Error writing to " << host << ": " << ec.message() << std::endl;
			return;
		}
		else if (request.size() != transferred) {
			request.erase(0, transferred);
			boost::asio::async_write(socket, boost::asio::buffer(request),
				[this](boost::system::error_code ec, size_t transferred) {
					write_handler(ec, transferred);
				});
		}
		else {
			boost::asio::async_read(socket, boost::asio::dynamic_buffer(response),
				[this](boost::system::error_code ec, size_t transferred) {
					read_handler(ec, transferred);
				});
		}
	}
	void read_handler(boost::system::error_code ec, size_t transverred) {
		if (ec && ec.value() != 2) {
			std::cerr << "Error reading from " << host << ": "
				<< ec.message() << std::endl;
		}
		else if (ec != boost::asio::error::eof) {
			std::cerr << "Error " << ec.message() << std::endl;
		}
	}
	const std::string& get_response() noexcept {
		return response;
	}

private:
	boost::asio::ip::tcp::resolver resolver;
	boost::asio::ip::tcp::socket socket;
	std::string request, response;
	const std::string host;
	int device_id;
};

int main() {
	setlocale(LC_ALL, "ru");
	const int num_clients = 4;
	boost::asio::io_context io_context;
	std::vector<std::unique_ptr<Request>> clients;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_clients; i++) {
		clients.emplace_back(std::make_unique<Request>(io_context, "localhost", i+50));
	}
	
	for (int i = 0; i < 4; i++) {
		threads.emplace_back([&io_context] {
			io_context.run();
			});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	for (auto& client : clients) {
		std::cout << client->get_response();
	}
}