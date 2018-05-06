#pragma once
#include <cstring>
#include <cstdlib>

class chat_message
{
public:
	enum { header_length = 4 };
	enum { max_body_length = 512};
	std::size_t length() const { return header_length + body_length_; }
	
	const char* data() const { return data_; }
	char* data() { return data_; }

	const char* body() const { return data_ + header_length; }
	char* body() { return data_ + header_length; }

	std::size_t body_length() { return body_length_; }
	void body_length(std::size_t new_length);

	void encode_header();
	bool decode_header();

private:
	std::size_t body_length_;
	char data_[header_length + max_body_length];
};
