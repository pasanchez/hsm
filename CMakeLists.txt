cmake_minimum_required(VERSION 3.7)
project(HSM)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp
                simpleHttpServer/Server.cpp
                simpleHttpServer/Server.h
                json/json-forwards.h
                json/json.h
                jsoncpp.cpp)
include_directories(/usr/include)
link_directories(/usr/lib)
add_executable(HSM ${SOURCE_FILES})
target_link_libraries(HSM sqlite3 boost_system)