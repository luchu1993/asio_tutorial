#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

class echo_client
{
public:
	echo_client(boost::asio::io_context& io_context, tcp::resolver::results_type const& endpoints)
		: io_context_(io_context)
		, socket_(io_context)
	{
		do_connect(endpoints);
	}

	void write(std::string const& msg)
	{
		write_msg_ = msg;
		boost::asio::async_write(socket_,
		boost::asio::buffer(write_msg_),
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				// nothing to do.		
			}
			else
			{
				socket_.close();
			}
		}
		);
	}

	void close()
	{
		io_context_.post([this]() { socket_.close(); });
	}

private:
	boost::asio::io_context& io_context_;
	tcp::socket socket_;
	std::string read_msg_;
	std::string write_msg_;

	void do_connect(tcp::resolver::results_type const& endpoints)
	{
		boost::asio::async_connect(socket_, endpoints, 
		[this] (boost::system::error_code ec, tcp::endpoint) 
		{
			if (!ec)
			{
				do_read();
			}
			else
			{
				socket_.close();
			}
		}
		);
	}

	void do_read()
	{
		boost::asio::async_read_until(socket_,
		boost::asio::dynamic_buffer(read_msg_), '\n',
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "Received: " << read_msg_;
				read_msg_.clear();
				do_read();
			}
			else
			{
				socket_.close();
			}
		}
		);
	}
};

int main(int argc, char * argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cout << "Usage: client <host> <port>\n";
			return 1;
		}

		boost::asio::io_context io_context;
		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve({ argv[1], argv[2] });
		echo_client c(io_context, endpoints);
		std::thread t([&io_context]() {io_context.run(); });

		char line[max_length + 1] = "";
		while (std::cin.getline(line, max_length))
		{
			line[std::strlen(line)] = '\n';
			std::cout << "Sent: " << line;
			c.write(line);
			std::memset(line, 0, max_length + 1);
		}

		c.close();
		t.join();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}