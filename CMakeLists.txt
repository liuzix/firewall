cmake_minimum_required(VERSION 3.5)
project(firewall)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
SET(CMAKE_C_FLAGS "-O3 -g")
file(GLOB UTILS
        "Utils/*.c"
        )
set(SOURCE_FILES tests.c firewall.c ${UTILS} myqueue.c myqueue.h lock.c lock.h TAS.c exp.c)

add_executable(firewall ${SOURCE_FILES})
target_link_libraries(firewall pthread m)