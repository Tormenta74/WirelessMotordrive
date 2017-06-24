cmake_minimum_required(VERSION 2.8)

project(console C)

if(EXISTS /usr/include/xbee.h)

    message(STATUS "xbee.h found")

    if(EXISTS /usr/lib/libxbee.so)

        message(STATUS "libxbee.so found")

        file(GLOB SRC "src/*.c")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lxbee -lpthread -lrt")

        include_directories(include /usr/include)
        add_executable(console ${SRC})
        target_link_libraries(console /usr/lib/libxbee.so)

    else(EXISTS /usr/lib/libxbee.so)

        message(STATUS "[ERROR] libxbee.so not found!")

    endif(EXISTS /usr/lib/libxbee.so)

else(EXISTS /usr/include/xbee.h)

    message(STATUS "[ERROR] xbee.h not found!")

endif(EXISTS /usr/include/xbee.h)

