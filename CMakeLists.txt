cmake_minimum_required(VERSION 3.24)

project(AstralBarrage VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
set(RAYLIB_VERSION 6.0)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)
FetchContent_MakeAvailable(raylib)

add_executable(AstralBarrage src/main.cpp)
target_link_libraries(AstralBarrage PRIVATE raylib)
