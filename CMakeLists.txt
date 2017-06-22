cmake_minimum_required (VERSION 3.0.2)
project (openvr-input-recorder)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11 /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11 /MTd")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_library(INPUT_EMULATOR_LIBRARIES
	NAMES
		libvrinputemulator
	PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator
	PATH_SUFFIXES
		Debug/lib/x86
		Debug/lib/x64
		Release/lib/x86
		Release/lib/x64
	NO_DEFAULT_PATH
	NO_CMAKE_FIND_ROOT_PATH
)

set(INPUT_EMULATOR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator/lib_vrinputemulator/include)

#add_definitions( -DBOOST_ALL_NO_LIB )
#set( Boost_USE_STATIC_LIBS ON )
#set(BOOST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator/third-party/boost_1_63_0)
#set(BOOST_LIBRARYDIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator/third-party/boost_1_63_0/lib64-msvc-14.0)
set(BOOST_ROOT C:/boost_1_63_0)
find_package(Boost REQUIRED)

find_library(OPENVR_LIBRARIES
  NAMES
    openvr_api
  PATHS
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/bin
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/lib
  PATH_SUFFIXES
    win64
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)
set(OPENVR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/headers)

SET(Protobuf_PROTOC_EXECUTABLE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x64-Release/Release/protoc.exe)
SET(Protobuf_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x64-Release/Release/libprotobuf.lib)
SET(Protobuf_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/src)
SET(Protobuf_LIBRARY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/protobuf/cmake/build/x64-Debug/Debug/libprotobufd.lib)

find_package(Protobuf REQUIRED)

file(GLOB_RECURSE openvr_input_recorder_sources "src/*.cc")
MESSAGE( STATUS "LIB: " ${OPENVR_INCLUDE_DIR})
add_executable(openvr-input-recorder ${openvr_input_recorder_sources} ${PROTO_HEADER} ${PROTO_SRC})
target_include_directories(openvr-input-recorder SYSTEM PUBLIC
  src/
  ${OPENVR_INCLUDE_DIR}
  ${PROTOBUF_INCLUDE_DIRS}
  ${INPUT_EMULATOR_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
)
target_link_libraries(openvr-input-recorder 
	debug ${OPENVR_LIBRARIES} ${Protobuf_LIBRARY_DEBUG} ${INPUT_EMULATOR_LIBRARIES}  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator/third-party/boost_1_63_0/lib64-msvc-14.0/libboost_date_time-vc140-mt-sgd-1_63.lib
	optimized ${OPENVR_LIBRARIES} ${PROTOBUF_LIBRARY} ${INPUT_EMULATOR_LIBRARIES}  ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/OpenVR-InputEmulator/third-party/boost_1_63_0/lib64-msvc-14.0/libboost_date_time-vc140-mt-sgd-1_63.lib
)

add_custom_command(TARGET openvr-input-recorder POST_BUILD 
               COMMAND ${CMAKE_COMMAND} -E copy_if_different
               "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/openvr/bin/win64/openvr_api.dll"              
                $<TARGET_FILE_DIR:openvr-input-recorder>)