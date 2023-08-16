
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

target_include_directories(${PROJECT_NAME}
    PUBLIC include
)