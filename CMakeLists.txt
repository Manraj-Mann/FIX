cmake_minimum_required(VERSION 3.16)
project(FIXEngineBenchmark VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -pipe")

add_executable(fix_benchmark
  src/main.cpp
  src/engine.cpp
)

target_include_directories(fix_benchmark PRIVATE src)
