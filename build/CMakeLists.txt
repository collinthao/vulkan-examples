cmake_minimum_required(VERSION 3.22)

project(vulkan_examples)

set(CMAKE_CXX_STANDARD 23)

find_package(assimp CONFIG REQUIRED)

add_executable(vulkan_examples ../src/app/main.cpp)

target_link_libraries(vulkan_examples PRIVATE assimp::assimp)
