project(app)

FILE(GLOB app_sources *.c)
target_sources(app PRIVATE ${app_sources})

#target_sources(${PROJECT_NAME} PRIVATE main.c)

# linking custom libraries causes a incomptabile CPU 
# architectures error for some reason
#
#target_link_libraries(${PROJECT_NAME}
#    PRIVATE drivers
#)
