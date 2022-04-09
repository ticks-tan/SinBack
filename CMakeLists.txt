cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack_Test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(SINBACK_OPENSSL true)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(${PROJECT_SOURCE_DIR}/lib)

file(GLOB_RECURSE SinBack_Base
        "${PROJECT_SOURCE_DIR}/lib/base/*.h"
        "${PROJECT_SOURCE_DIR}/lib/base/*.cc")
# core module
file(GLOB_RECURSE SinBack_Core
        "${PROJECT_SOURCE_DIR}/lib/core/*.h"
        "${PROJECT_SOURCE_DIR}/lib/core/*.cc")
# module module
file(GLOB_RECURSE SinBack_Module
        "${PROJECT_SOURCE_DIR}/lib/module/*.h"
        "${PROJECT_SOURCE_DIR}/lib/module/*.cc"
        "${PROJECT_SOURCE_DIR}/lib/module/http/*.h"
        "${PROJECT_SOURCE_DIR}/lib/module/http/*.cc")
# tool module
file(GLOB_RECURSE SinBack_Tools
        "${PROJECT_SOURCE_DIR}/lib/tools/*.h"
        "${PROJECT_SOURCE_DIR}/lib/tools/*.cc")
# Application
file(GLOB_RECURSE SinBack_Main
        "${PROJECT_SOURCE_DIR}/lib/*.h"
        "${PROJECT_SOURCE_DIR}/lib/*.cc")

add_library(SinBack_Shared SHARED
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Module}
        ${SinBack_Tools}
        ${SinBack_Main})

#add_library(SinBack_Static STATIC
#        ${SinBack_Base}
#        ${SinBack_Core}
#        ${SinBack_Net}
#        ${SinBack_Tools})

add_executable(SinBack_Test
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Module}
        ${SinBack_Tools}
        ${SinBack_Main})

set_target_properties(SinBack_Shared PROPERTIES OUTPUT_NAME "SinBack")
# set_target_properties(SinBack_Static PROPERTIES OUTPUT_NAME "SinBack")

set_target_properties(SinBack_Shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
# set_target_properties(SinBack_Static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set_target_properties(SinBack_Shared PROPERTIES VERSION 1.0 SOVERSION 1)

# openssl
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "OpenSSL Found")
endif()

link_directories(/usr/lib)
target_link_libraries(SinBack_Shared pthread llhttp OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(SinBack_Test pthread llhttp OpenSSL::Crypto OpenSSL::SSL)


