cmake_minimum_required(VERSION 3.10)

include_directories("C:/Boost/include/boost-1_67")
link_directories("C:/Boost/lib")

# echo example
add_executable(echo_async_client echo_async_client.cpp)
add_executable(echo_sync_client echo_sync_client.cpp)
add_executable(echo_async_server echo_async_server.cpp)