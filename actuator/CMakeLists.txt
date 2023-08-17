# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(Actuator
        VERSION 1.0
        DESCRIPTION "Servo actuator"
        LANGUAGES C
)

add_subdirectory(drivers)
add_subdirectory(src)