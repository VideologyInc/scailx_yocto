# SPDX-License-Identifier: MIT

project(serial-xfer C)
cmake_minimum_required(VERSION 2.6)
add_executable(serial-xfer serial-xfer.c)
target_link_libraries(serial-xfer rt)
# make sure output is optimised
set(CMAKE_BUILD_TYPE Release)
install(TARGETS serial-xfer DESTINATION bin)