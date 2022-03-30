cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack_Test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(${PROJECT_SOURCE_DIR}/lib)

# http-parse module
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/llhttp)

file(GLOB_RECURSE SinBack_Base
        "${PROJECT_SOURCE_DIR}/lib/base/*.h"
        "${PROJECT_SOURCE_DIR}/lib/base/*.cc")
# core module
file(GLOB_RECURSE SinBack_Core
        "${PROJECT_SOURCE_DIR}/lib/core/*.h"
        "${PROJECT_SOURCE_DIR}/lib/core/*.cc")
# net module
file(GLOB_RECURSE SinBack_Net
        "${PROJECT_SOURCE_DIR}/lib/net/*.h"
        "${PROJECT_SOURCE_DIR}/lib/net/*.cc"
        "${PROJECT_SOURCE_DIR}/lib/net/http/*.h"
        "${PROJECT_SOURCE_DIR}/lib/net/http/*.cc")

# tool module
file(GLOB_RECURSE SinBack_Tools
        "${PROJECT_SOURCE_DIR}/lib/tools/*.h"
        "${PROJECT_SOURCE_DIR}/lib/tools/*.cc")

add_library(SinBack_Shared SHARED
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Net}
        ${SinBack_Tools})

add_library(SinBack_Static STATIC
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Net}
        ${SinBack_Tools})

add_executable(SinBack_Test
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Net}
        ${SinBack_Tools}
        lib/main.cc)

set_target_properties(SinBack_Shared PROPERTIES OUTPUT_NAME "SinBack")
set_target_properties(SinBack_Static PROPERTIES OUTPUT_NAME "SinBack")

set_target_properties(SinBack_Shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(SinBack_Static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set_target_properties(SinBack_Shared PROPERTIES VERSION 1.0 SOVERSION 1)

# link [ pthread ]
link_libraries(pthread.a)
target_link_libraries(SinBack_Test ${PROJECT_SOURCE_DIR}/build/lib/libllhttp.a)
target_link_libraries(SinBack_Shared ${PROJECT_SOURCE_DIR}/build/lib/libllhttp.a)
target_link_libraries(SinBack_Static ${PROJECT_SOURCE_DIR}/build/lib/libllhttp.a)


