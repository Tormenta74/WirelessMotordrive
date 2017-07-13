#
# CMakeLists file for the console application build.
#
# This file is part of the Wireless Motordrive project.
# Copyright (C) 2017 Diego Sáinz de Medrano.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, visit https://www.gnu.org/licenses/ to get
# a copy.


cmake_minimum_required(VERSION 2.8)

project(console CXX)

if(EXISTS /usr/include/xbee.h)

    message(STATUS "xbee.h found")

    if(EXISTS /usr/lib/libxbee.so)

        message(STATUS "libxbee.so found")

        file(GLOB SRC "src/*.cpp")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
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

