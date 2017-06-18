cmake_minimum_required (VERSION 3.0.2)
project (openvr-input-recorder)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 /MTd")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_library(OPENVR_LIBRARIES
  NAMES
    openvr_api
  PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/bin
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/lib
  PATH_SUFFIXES
    osx32
    linux64
    win32
    win64
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)
set(OPENVR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/headers)

SET(Protobuf_PROTOC_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x86-Release/Release/protoc.exe)
SET(Protobuf_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x86-Release/Release/libprotobuf.lib)
SET(Protobuf_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/src)
SET(Protobuf_LIBRARY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x86-Debug/Debug/libprotobufd.lib)

INCLUDE(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(
  src/
  ${OPENVR_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIRS}
)

file(GLOB_RECURSE openvr_input_recorder_sources "src/*.cc")

add_executable(openvr-input-recorder ${openvr_input_recorder_sources} ${PROTO_HEADER} ${PROTO_SRC})
target_link_libraries(openvr-input-recorder 
	debug ${OPENVR_LIBRARIES} ${Protobuf_LIBRARY_DEBUG}
	optimized ${OPENVR_LIBRARIES} ${PROTOBUF_LIBRARY}
)

add_custom_command(TARGET openvr-input-recorder POST_BUILD 
               COMMAND ${CMAKE_COMMAND} -E copy_if_different
               "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/bin/win32/openvr_api.dll"              
                $<TARGET_FILE_DIR:openvr-input-recorder>)