cmake_minimum_required(VERSION 3.10)

include_directories("C:/Boost/include/boost-1_67")
link_directories("C:/Boost/lib")

# char room examples
add_executable(chat_async_client chat_async_client.cpp chat_message.cpp chat_message.h)
add_executable(chat_async_server chat_async_server.cpp chat_message.cpp chat_message.h)