cmake_minimum_required(VERSION 3.20)
project(TapServer)

set(CMAKE_CXX_STANDARD 17)

add_executable(TapServer main.cpp RPGServer.cpp RPGServer.h RPGServer.h RPGClient.cpp RPGClient.h DS.cpp DS.h GameClient.cpp GameClient.h)

find_package(Threads REQUIRED)
target_link_libraries(TapServer pthread)
