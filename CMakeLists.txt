cmake_minimum_required(VERSION 3.16)
project(StickerInspect C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include(${PROJECT_SOURCE_DIR}/cmake/platform.cmake)
include_directories(src/dll)
include_directories(src)

# Include all vendor libraries, also sets up HL2SDK and Metamod
add_subdirectory(vendor)

# Compile protobufs

include(${PROJECT_SOURCE_DIR}/cmake/protobuf.cmake)

# Source
add_subdirectory(src)