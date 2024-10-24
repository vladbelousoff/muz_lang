cmake_minimum_required(VERSION 3.29)
project(muz_lang C)

set(CMAKE_C_STANDARD 99)

file(GLOB_RECURSE HEADER_FILES "${PROJECT_NAME}/*.h")
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_NAME}/*.c")

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

add_executable(${PROJECT_NAME} main.c ${HEADER_FILES} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PUBLIC
        MUZ_DEBUG_LEVEL=4)
