cmake_minimum_required(VERSION 3.25)
project(REST)

set(CMAKE_CXX_STANDARD 23)

find_package(cpprestsdk CONFIG REQUIRED)


add_executable(REST main.cpp)
target_link_libraries(REST PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)
