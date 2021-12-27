cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pulse_gen_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

file(GLOB SOURCES
    *.c
    *.h
)
add_executable(pulse_gen
    ${SOURCES}
)
pico_enable_stdio_usb(pulse_gen 1)
pico_add_extra_outputs(pulse_gen)
target_link_libraries(pulse_gen pico_stdlib)