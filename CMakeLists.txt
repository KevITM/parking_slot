cmake_minimum_required(VERSION 3.10)
project(ParkingLotSystem)

set(CMAKE_CXX_STANDARD 17)

# Server dynamic library
add_library(parking_server_lib SHARED src/server_lib/receiver.cpp)
if(WIN32)
    target_link_libraries(parking_server_lib ws2_32)
endif()

# Client executable
add_executable(generator_client src/client/main.cpp)
if(WIN32)
    target_link_libraries(generator_client ws2_32)
endif()
