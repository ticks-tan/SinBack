cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)

include_directories(${PROJECT_SOURCE_DIR}/lib)

# http-parse module
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/llhttp)
# fmt module
file(GLOB_RECURSE lib_fmt "${PROJECT_SOURCE_DIR}/lib/libfmt/*.h" "${PROJECT_SOURCE_DIR}/lib/libfmt/*.cc")
# base module
file(GLOB_RECURSE SinBack_Base "${PROJECT_SOURCE_DIR}/lib/base/*.h" "${PROJECT_SOURCE_DIR}/lib/base/*.cc")
# core module
file(GLOB_RECURSE SinBack_Core "${PROJECT_SOURCE_DIR}/lib/core/*.h" "${PROJECT_SOURCE_DIR}/lib/core/*.cc")
# net module
file(GLOB_RECURSE SinBack_Net "${PROJECT_SOURCE_DIR}/lib/net/*.h" "${PROJECT_SOURCE_DIR}/lib/net/*.cc")

# link [ pthread ]
link_libraries(pthread)

add_executable(SinBack
        ${lib_fmt}
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Net}
        lib/main.cc)

target_link_libraries(SinBack llhttp)
