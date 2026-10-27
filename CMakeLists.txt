cmake_minimum_required(VERSION 3.10)
project(VAAW_VM_C C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
    .
    Decoder
    VirtualMachine
)

add_executable(VAAW_VM_C
    main.c
    Decoder/Decoder.c
    VirtualMachine/VM.c
)