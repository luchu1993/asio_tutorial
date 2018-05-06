#include "chat_message.h"
#include <cstdio>

void chat_message::body_length(std::size_t new_length)
{
	body_length_ = new_length;
	if (body_length_ > max_body_length)
		body_length_ = max_body_length;
}

void chat_message::encode_header()
{
	char header[header_length + 1] = "";
	std::sprintf(header, "%4d", body_length_);
	std::memcpy(data_, header, 
		static_cast<std::size_t>(std::strlen(header)));
}

bool chat_message::decode_header()
{
	char header[header_length + 1] = "";
	std::strncpy(header, data_, header_length);
	body_length_ = std::atoi(header);
	if (body_length_ > max_body_length)
	{
		body_length_ = 0;
		return false;
	}
	return true;
}

