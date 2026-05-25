cmake_minimum_required(VERSION 3.10)
project(SpaceWar CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch SDL2
FetchContent_Declare(
    SDL2
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.30.9.zip
)
FetchContent_MakeAvailable(SDL2)

if(TARGET SDL2::SDL2main)
    set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
else()
    set(SDL2_LIBRARIES SDL2::SDL2)
endif()

# Fetch ZLIB (needed for network pako deflate/inflate equivalent)
FetchContent_Declare(
    zlib
    URL https://github.com/madler/zlib/archive/refs/tags/v1.3.1.zip
)
FetchContent_MakeAvailable(zlib)
set(ZLIB_LIBRARIES zlibstatic)

set(USE_TLS OFF CACHE BOOL "Disable TLS in ixwebsocket" FORCE)
set(USE_ZLIB OFF CACHE BOOL "Disable internal ZLIB in ixwebsocket" FORCE)
FetchContent_Declare(
    ixwebsocket
    URL https://github.com/machinezone/IXWebSocket/archive/refs/tags/v11.4.5.zip
)
FetchContent_MakeAvailable(ixwebsocket)

file(GLOB SOURCES "*.cpp")

add_executable(SpaceWar ${SOURCES})

# Target includes and libraries
# Let FetchContent handles the include dirs automatically via target_link_libraries
target_link_libraries(SpaceWar PRIVATE ${SDL2_LIBRARIES} ${ZLIB_LIBRARIES} ixwebsocket)
