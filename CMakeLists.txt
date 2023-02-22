PROJECT(protobuf-rpc-demp)
cmake_minimum_required(VERSION 3.2)
add_compile_options(-std=c++11)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/output/bin")

include_directories(
    /usr/local/Cellar/boost/1.79.0_1/include
    /Users/fengyuwei/code/cpp/protobuf-3.6.1/output/include
        /usr/local/include/
    )
link_directories(
    /Users/fengyuwei/code/cpp/protobuf-3.6.1/output/lib
    /usr/local/lib
    ./build)
    
file(GLOB_RECURSE RPC_SOURCES "${PROJECT_SOURCE_DIR}/src/myrpc/*.cc")

#add_library(myrpc src/myrpc/event_loop.cc src/myrpc/event_loop.h src/myrpc/selector.cc src/myrpc/selector.h src/myrpc/tcp_conn.cc src/myrpc/tcp_conn.h src/myrpc/rpc_conn.cc src/myrpc/rpc_conn.h src/myrpc/tcp_server.cc src/myrpc/tcp_server.h src/myrpc/rpc_server.cc src/myrpc/rpc_server.h)
add_library(myrpc src/myrpc/rpc_meta.pb.cc src/myrpc/event_loop.cc src/myrpc/event_loop.h src/myrpc/selector.cc src/myrpc/selector.h src/myrpc/tcp_conn.cc src/myrpc/tcp_conn.h src/myrpc/tcp_server.cc src/myrpc/tcp_server.h src/myrpc/rpc_server.cc src/myrpc/rpc_server.h src/myrpc/rpc_client.cc src/myrpc/rpc_client.h)
#target_link_libraries(tcp_server libprotobuf.a libboost_system.a libglog.dylib)

file(GLOB_RECURSE MY_SOURCES "${PROJECT_SOURCE_DIR}/*.cc")
file(GLOB_RECURSE MY_HEADERS "${PROJECT_SOURCE_DIR}/*.h")

include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/myrpc)

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

add_executable(server ./src/echo.pb.cc ./src/server.cc)
target_link_libraries(server libprotobuf.a libboost_system.a myrpc libglog.dylib)
add_executable(client ./src/echo.pb.cc ./src/client.cc)
target_link_libraries(client libprotobuf.a libboost_system.a myrpc libglog.dylib)

add_executable(test src/test.cc src/test.h)
target_link_libraries(test libprotobuf.a libboost_system.a myrpc libglog.dylib)

file(COPY ${PROJECT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/output/include/
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        )
