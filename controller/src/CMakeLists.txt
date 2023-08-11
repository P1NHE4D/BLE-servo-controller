project(app)


target_sources(${PROJECT_NAME} PRIVATE main.c)


target_link_libraries(${PROJECT_NAME}
    PRIVATE drivers
)
