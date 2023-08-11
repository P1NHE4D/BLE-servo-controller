project(drivers)

add_library(${PROJECT_NAME} src/status_led.c)

target_include_directories(${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)