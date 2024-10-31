cmake_minimum_required(VERSION 3.16)
project(fk)

# set(CMAKE_BUILD_TYPE Debug)

# source directory
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# Specify C++ standard
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# Optionally set compiler-specific flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -g -Werror=return-type)
endif()

# For multi-generator support
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()
