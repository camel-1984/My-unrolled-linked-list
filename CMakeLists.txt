cmake_minimum_required(VERSION 3.12)

project(
    my-unrolled-linked-list
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

include_directories(lib)

enable_testing()
add_subdirectory(tests)
