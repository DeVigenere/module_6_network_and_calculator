cmake_minimum_required(VERSION 3.10)
project(network VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32_LEAN_AND_MEAN")
endif()

set(SOURCES
    Calculator.cpp
)

set(HEADERS
    Calculator.h
)

add_executable(calculator_sender
    calculator_sender.cpp
    ${SOURCES}
    ${HEADERS}
)

if (WIN32)
    target_link_libraries(calculator_sender ws2_32)
endif()

add_executable(receiver_server
    receiver.cpp
)

if (WIN32)
    target_link_libraries(receiver_server ws2_32)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

if (WIN32)
    message(STATUS "Platform: Windows (linking ws2_32)")
else()
    message(STATUS "Platform: Unix/Linux")
endif()