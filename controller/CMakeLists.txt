# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Controller
        VERSION 1.0
        DESCRIPTION "Servo controller"
        LANGUAGES C
)

add_subdirectory(lib)
add_subdirectory(src)