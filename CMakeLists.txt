cmake_minimum_required(VERSION 3.10)
project(FIX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable logging
# add_definitions(-DENABLE_LOGGING)

# store the executable in bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(learnings)