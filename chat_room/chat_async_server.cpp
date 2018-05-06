#include "chat_message.h"

#include <iostream>
#include <memory>
#include <list>
#include <deque>
#include <set>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class chat_participant
{
public:
	virtual ~chat_participant() { }
	virtual void deliver(chat_message const& msg) = 0;
};

using participant_ptr = std::shared_ptr<chat_participant>;
using chat_message_queue = std::deque<chat_message>;

class chat_room
{
public:
	void join(participant_ptr const& participant)
	{
		participants_.insert(participant);
		for (auto const& msg : recent_msgs_)
		{
			participant->deliver(msg);
		}
	}

	void leave(participant_ptr const& participant)
	{
		participants_.erase(participant);
	}

	void deliver(chat_message const& msg)
	{
		recent_msgs_.push_back(msg);
		while (recent_msgs_.size() > max_recent_size)
			recent_msgs_.pop_front();
		for (auto const& participant : participants_)
		{
			participant->deliver(msg);
		}
	}

private:
	enum { max_recent_size = 100 };
	std::set<participant_ptr> participants_;
	chat_message_queue recent_msgs_;
};


class chat_session
	: public std::enable_shared_from_this<chat_session>
	, public chat_participant
{
public:
	chat_session(tcp::socket socket, chat_room& room)
		: socket_(std::move(socket))
		, room_(room)
	{ }

	void start()
	{
		room_.join(shared_from_this());
		do_read_header();
	}

	void deliver(chat_message const& msg) override
	{
		bool write_in_process = !write_msgs_.empty();
		write_msgs_.push_back(msg);
		if (!write_in_process)
		{
			do_write();
		}
	}

private:
	void do_read_header()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.data(), chat_message::header_length),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec && read_msg_.decode_header())
			{
				do_read_body();
			}
			else
			{
				room_.leave(shared_from_this());
			}
		}
		);
	}

	void do_read_body()
	{
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
		boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				// print received message for debugging
				std::cout << "[Log] Received message: ";
				std::cout.write(read_msg_.body(), read_msg_.body_length());
				std::cout << std::endl;
				room_.deliver(read_msg_);
				do_read_header();
			}
			else
			{
				room_.leave(shared_from_this());
			}
		}
		);
	}

	void do_write()
	{
		auto self(shared_from_this());
		boost::asio::async_write(socket_,
		boost::asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				write_msgs_.pop_front();
				if (!write_msgs_.empty())
					do_write();
			}
			else
			{
				room_.leave(shared_from_this());
			}
		}
		);
	}

	tcp::socket socket_;
	chat_room& room_;
	chat_message read_msg_;
	chat_message_queue write_msgs_;
};


class chat_server
{
public:
	chat_server(boost::asio::io_context& io_context,
		tcp::endpoint const & endpoint)
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
				std::cout << "[Log] Connection from " << socket.remote_endpoint().address().to_string()
					<< ", port " << socket.remote_endpoint().port() << std::endl;

				std::make_shared<chat_session>(std::move(socket), room)->start();
			}
			do_accept();
		}
		);
	}

	chat_room room;
	tcp::acceptor acceptor_;
};


int main(int argc, char *argv[])
{
	try
	{
		if (argc < 2)
		{
			std::cerr << "Usage: server <port> [<port> ...]" << std::endl;
			return 1;
		}

		boost::asio::io_context io_context;
		std::list<chat_server> servers;
		for (int i = 1; i < argc; ++i)
		{
			servers.emplace_back(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
		}
		io_context.run();
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
	}
}