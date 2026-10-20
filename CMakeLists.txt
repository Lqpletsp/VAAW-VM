cmake_minimum_required(VERSION 3.15)

project(ASMMICVM VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Decoder
)

set(SOURCES
    main.cpp
    VirtualMachine/VM.cpp
    Decoder/Decoder.cpp
)

add_executable(VAAWVM ${SOURCES})
