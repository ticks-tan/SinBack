cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack CXX)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 11)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)

include_directories(${PROJECT_SOURCE_DIR}/lib)

# fmt module
file(GLOB_RECURSE lib_fmt "${PROJECT_SOURCE_DIR}/lib/libfmt/*.h" "${PROJECT_SOURCE_DIR}/lib/libfmt/*.cc")
# base module
file(GLOB_RECURSE SinBack_Base "${PROJECT_SOURCE_DIR}/lib/base/*.h" "${PROJECT_SOURCE_DIR}/lib/base/*.cc")
# core module
file(GLOB_RECURSE SinBack_Core "${PROJECT_SOURCE_DIR}/lib/core/*.h" "${PROJECT_SOURCE_DIR}/lib/core/*.cc")

link_libraries(pthread)

add_executable(SinBack
        ${lib_fmt}
        ${SinBack_Base}
        ${SinBack_Core}
        lib/main.cc)
