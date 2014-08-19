cmake_minimum_required(VERSION 3.0)
project(NetOCE)

set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/out)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OCE)
find_package(Protobuf)
include(deps/CMakeLists.libuv.txt)

execute_process(COMMAND node ${CMAKE_SOURCE_DIR}/genhandlers.js ${CMAKE_BINARY_DIR}/handler.h)

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${OCE_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${LIBUV_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
)

protobuf_generate_cpp(ProtoSources ProtoHeaders ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/net-oce-protocol/oce.proto)



add_executable(
  net-oce
  main.cpp
  editor.cpp
  ${ProtoHeaders}
  ${ProtoSources}
)


target_link_libraries (net-oce ${OCE_LIBRARIES} ${PROTOBUF_LIBRARIES} uv)

