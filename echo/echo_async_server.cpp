#include <iostream>
#include <memory>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class echo_session
	: public std::enable_shared_from_this<echo_session>
{
public:
	echo_session(tcp::socket socket)
		: socket_(std::move(socket))
	{ }

	void start() { do_read(); }

private:
	void do_read()
	{
		auto self(shared_from_this());

		socket_.async_read_some(
		boost::asio::buffer(data_, max_length),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "[Log] Received: ";
				std::cout.write(data_, length);
				std::cout << std::endl;
				do_write(length);
			}
		}
		);
	}
	void do_write(std::size_t length)
	{
		auto self(shared_from_this());

		boost::asio::async_write(socket_,
		boost::asio::buffer(data_, length),
		[this, self](boost::system::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		}
		);
	}

	tcp::socket socket_;
	enum { max_length = 512 };
	char data_[max_length];
};

class echo_server
{
public:
	echo_server(boost::asio::io_context& io_context, tcp::endpoint const& endpoint)
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
				std::cout << "[Log] Connected from: " << socket.remote_endpoint().address().to_string()
					<< ", port " << socket.remote_endpoint().port() << std::endl;

				std::make_shared<echo_session>(std::move(socket))->start();
			}

			do_accept();
		}
		);
	}

	tcp::acceptor acceptor_;
};

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cout << "Usage: server <port>" << std::endl;
			return 1;
		}

		boost::asio::io_context io_context;
		echo_server server(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}