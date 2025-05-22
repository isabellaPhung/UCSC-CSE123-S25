cmake_minimum_required(VERSION 3.10)

# Set the project name
project(caldata
        VERSION 0.9
        DESCRIPTION "Database scripts"
        LANGUAGES C)

find_package(SQLite3 REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Include directories
include_directories(${SQLite3_INCLUDE_DIRS})

# Add the main executable
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.c")
add_executable(test_task_db tests/SQLtests.c ${SOURCES})

# Link libraries to the main executable
target_link_libraries(test_task_db ${SQLite3_LIBRARIES})