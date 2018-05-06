cmake_minimum_required(VERSION 3.10)

include_directories("C:/Boost/include/boost-1_67")
link_directories("C:/Boost/lib")

# daytime examples
add_executable(daytime_sync_client daytime_sync_client.cpp)
add_executable(daytime_async_server daytime_async_server.cpp)