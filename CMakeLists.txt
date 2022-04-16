cmake_minimum_required(VERSION 3.10~3.22)

PROJECT(SinBack_Test CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(SINBACK_OPENSSL true)
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install")

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
add_library(SinBack_Static STATIC
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Module}
        ${SinBack_Tools}
        ${SinBack_Main})

add_executable(SinBack_Test
        ${SinBack_Base}
        ${SinBack_Core}
        ${SinBack_Module}
        ${SinBack_Tools}
        ${SinBack_Main}
        main.cc)

set_target_properties(SinBack_Shared PROPERTIES OUTPUT_NAME "SinBack")
set_target_properties(SinBack_Static PROPERTIES OUTPUT_NAME "SinBack")

set_target_properties(SinBack_Shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(SinBack_Static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set_target_properties(SinBack_Shared PROPERTIES VERSION 1.0 SOVERSION 1)

# openssl
find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "OpenSSL Found")
endif()

target_link_libraries(SinBack_Shared pthread llhttp OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(SinBack_Static pthread llhttp OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(SinBack_Test pthread llhttp OpenSSL::Crypto OpenSSL::SSL)

file(GLOB_RECURSE INSTALL_BASE_HEADER ${PROJECT_SOURCE_DIR}/lib/base/*.h)
file(GLOB_RECURSE INSTALL_CORE_HEADER
        ${PROJECT_SOURCE_DIR}/lib/core/Event.h
        ${PROJECT_SOURCE_DIR}/lib/core/EventLoop.h
        ${PROJECT_SOURCE_DIR}/lib/core/Channel.h
        ${PROJECT_SOURCE_DIR}/lib/core/EventLoopPool.h
        ${PROJECT_SOURCE_DIR}/lib/core/EventLoopThread.h
        ${PROJECT_SOURCE_DIR}/lib/core/Selector.h)
file(GLOB_RECURSE INSTALL_MODULE_HEADER
        ${PROJECT_SOURCE_DIR}/lib/module/Module.h
        ${PROJECT_SOURCE_DIR}/lib/module/TcpServer.h
        ${PROJECT_SOURCE_DIR}/lib/module/http/HttpContext.h
        ${PROJECT_SOURCE_DIR}/lib/module/http/HttpServer.h
        ${PROJECT_SOURCE_DIR}/lib/module/http/HttpParser.h
        ${PROJECT_SOURCE_DIR}/lib/module/http/HttpMessage.h
        ${PROJECT_SOURCE_DIR}/lib/module/http/HttpService.h)
file(GLOB_RECURSE INSTALL_MAIN_HEADER ${PROJECT_SOURCE_DIR}/lib/Application.h)
install(TARGETS SinBack_Shared
        RUNTIME DESTINATION SinBack/bin
        LIBRARY DESTINATION SinBack/lib
        ARCHIVE DESTINATION SinBack/lib)
install(TARGETS SinBack_Static
        RUNTIME DESTINATION SinBack/bin
        LIBRARY DESTINATION SinBack/lib
        ARCHIVE DESTINATION SinBack/lib)
install(FILES ${INSTALL_BASE_HEADER} DESTINATION SinBack/include/base)
install(FILES ${INSTALL_CORE_HEADER} DESTINATION SinBack/include/core)
install(FILES ${INSTALL_MODULE_HEADER} DESTINATION SinBack/include/module)
install(FILES ${INSTALL_MAIN_HEADER} DESTINATION SinBack/include)

