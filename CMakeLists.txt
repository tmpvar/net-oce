cmake_minimum_required(VERSION 3.0)
project(OCE-experiments)

# add_subdirectory(../oce "${CMAKE_CURRENT_BINARY_DIR}/oce")

# include(../oce/CMakeLists.txt)
# include_directories(../oce/inc ../oce/src ../oce/src/Standard ../oce/src/TCollection)

add_executable(net-oce bottle.cpp main.cpp transport/tcp.cpp)

find_package(OCE)
if (OCE_FOUND)
  include_directories(${OCE_INCLUDE_DIRS})
  target_link_libraries (net-oce ${OCE_LIBRARIES})
endif (OCE_FOUND)

find_package(Protobuf)
if (PROTOBUF_FOUND)
  include_directories(${PROTOBUF_INCLUDE_DIRS})
  target_link_libraries (net-oce ${PROTOBUF_LIBRARIES})
endif (PROTOBUF_FOUND)

target_link_libraries (net-oce uv)
find_path(LIBUV_INCLUDE_DIR uv.h)

include_directories(${LIBUV_INCLUDE_DIR})

