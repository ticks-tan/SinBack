cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

include_directories(${PROJECT_SOURCE_DIR}/lib)

file(GLOB_RECURSE lib_fmt "${PROJECT_SOURCE_DIR}/lib/fmt/*.h" "{${PROJECT_SOURCE_DIR}/lib/fmt/*.cc")
file(GLOB_RECURSE SinBack_Core "${PROJECT_SOURCE_DIR}/lib/core/*.h" "${PROJECT_SOURCE_DIR}/lib/core/*.cc")
file(GLOB_RECURSE SinBack_Base "${PROJECT_SOURCE_DIR}/lib/base/*.h" "${PROJECT_SOURCE_DIR}/lib/base/*.cc")

add_executable(SinBack
        ${SinBack_Base}
        ${SinBack_Core}
        ${lib_fmt}
        lib/main.cc)