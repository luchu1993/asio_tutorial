#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

enum { max_length = 512 };

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>\n";
			return 1;
		}

		boost::asio::io_context io_context;

		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve({ argv[1], argv[2] });
		tcp::socket socket(io_context);
		boost::system::error_code ec;
		// connect to server
		boost::asio::connect(socket, endpoints, ec);
		
		// request
		std::cout << "Enter message: ";
		char request[max_length];

		while (std::cin.getline(request, max_length))
		{
			std::size_t request_length = std::strlen(request);
			boost::asio::write(socket, boost::asio::buffer(request, request_length));

			char reply[max_length];
			std::size_t reply_length = boost::asio::read(socket, boost::asio::buffer(reply, request_length));
			std::cout << "Reply is: ";
			std::cout.write(reply, reply_length);
			std::cout << "\n";

			std::cout << "Enter message: ";
		}		
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}