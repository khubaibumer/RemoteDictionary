cmake_minimum_required(VERSION 3.22)
project(RemoteDictionary)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json 3.10.5 REQUIRED)

add_executable(RemoteDictionaryServer
        server/src/main.cpp
        server/src/Server.cpp
        server/src/Dictionary.cpp
        server/src/Thread.cpp
        server/src/ThreadPool.cpp
        server/src/Stats.cpp)

add_executable(RemoteDictionaryClient
        client/src/main.cpp
        client/src/Client.cpp)
