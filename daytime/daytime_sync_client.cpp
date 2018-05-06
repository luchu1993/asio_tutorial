#include <iostream>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

int main(int argc, char* argv[])
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

		// connected to server
		boost::asio::connect(socket, endpoints,ec);
		// read message from server
		std::string daytime_str;
		boost::asio::read_until(socket, boost::asio::dynamic_buffer(daytime_str), '\n', ec);
		std::cout << daytime_str;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}