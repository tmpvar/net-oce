cmake_minimum_required(VERSION 3.0)
project(OCE-experiments)

set(PROJECT_BINARY_DIR ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/out)
set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_BINARY_DIR})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

find_path(LIBUV_INCLUDE_DIR uv.h)
find_package(OCE)
find_package(Protobuf)

include_directories(
  ${OCE_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${LIBUV_INCLUDE_DIR}
  ${CMAKE_BINARY_DIR}
)

file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/protocol/*.proto")
protobuf_generate_cpp(ProtoSources ProtoHeaders ${ProtoFiles})

add_executable(
  net-oce
  main.cpp
  editor.cpp
  ${ProtoHeaders}
  ${ProtoSources}
)

target_link_libraries (net-oce ${OCE_LIBRARIES} ${PROTOBUF_LIBRARIES} uv)

