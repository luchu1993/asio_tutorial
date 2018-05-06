#include <iostream>
#include <memory>
#include <ctime>
#include <cstring>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

std::string get_system_daytime()
{
	using namespace std;
	time_t now = time(nullptr);
	return ctime(&now);
}

class session
	: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket)
		: socket_(std::move(socket))
	{ }

	void start()
	{
		time_str_ = get_system_daytime();

		auto self(shared_from_this());
		boost::asio::async_write(socket_,
		boost::asio::buffer(time_str_),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			socket_.close();
		}
		);
	}

private:
	tcp::socket socket_;
	std::string time_str_;
};

class daytime_server
{
public:
	daytime_server(boost::asio::io_context& io_context, tcp::endpoint const& endpoint)
		: acceptor_(io_context, endpoint)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(
			[this](boost::system::error_code ec, tcp::socket socket)
		{
			if (!ec)
			{
				// print connection information for debugging
				std::cout << "[Log] Connected from" << socket.remote_endpoint().address().to_string()
					<< ", port " << socket.remote_endpoint().port() << std::endl;
				std::make_shared<session>(std::move(socket))->start();
			}
			do_accept();
		}
		);
	}

	tcp::acceptor acceptor_;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			return 1;
		}

		boost::asio::io_context io_context;
		daytime_server server(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}
}