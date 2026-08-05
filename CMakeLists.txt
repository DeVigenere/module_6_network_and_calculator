cmake_minimum_required(VERSION 3.10)
project(network VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_executable(receiver_server
    receiver.cpp
)

