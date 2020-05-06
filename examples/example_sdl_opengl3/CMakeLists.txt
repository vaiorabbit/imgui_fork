# CMakeLists.txt for macOS
# Usage:
# - Put 'SDL2.framework' at imgui/examples/libs/SDL2 (Use imgui/examples/libs/SDL2/GetSDL2Framework.sh)
# - Put this file at imgui/examples/example_sdl_opengl3
# - $ cd imgui/examples/example_sdl_opengl3
# - $ mkdir build
# - $ cd build
# - $ cmake .. -G "Xcode"
# - $ open example_sdl_opengl3.xcodeproj

cmake_minimum_required( VERSION 3.17.1 )
project( example_sdl_opengl3 )

find_package(OpenGL)

# SDL2.framework (macOS)
if (CMAKE_HOST_APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_CURRENT_LIST_DIR}/../libs/SDL2")
	find_path(SDL2_INCLUDE NAMES "SDL.h")
    find_library(SDL2_LIBRARY NAMES SDL2)
endif()

set( EXAMPLE_SDL_OPENGL3_HEADERS
)

set( EXAMPLE_SDL_OPENGL3_SOURCES
    main.cpp
)

set(IMGUI_PATH "${CMAKE_CURRENT_LIST_DIR}/../..")
set(IMGUI_IMPL_PATH "${CMAKE_CURRENT_LIST_DIR}/..")
file( GLOB IMGUI_SOURCES
    "${IMGUI_PATH}/*.cpp"
    "${IMGUI_PATH}/*.h"
)
file( GLOB IMGUI_IMPL_SOURCES
	"${IMGUI_IMPL_PATH}/imgui_impl_opengl3.h"
	"${IMGUI_IMPL_PATH}/imgui_impl_opengl3.cpp"
	"${IMGUI_IMPL_PATH}/imgui_impl_sdl.h"
	"${IMGUI_IMPL_PATH}/imgui_impl_sdl.cpp"
)
source_group("imgui" FILES ${IMGUI_SOURCES})
source_group("imgui_impl" FILES ${IMGUI_IMPL_SOURCES})

set(GL3W_PATH "${CMAKE_CURRENT_LIST_DIR}/../libs/gl3w")
file( GLOB GL3W_SOURCES
    "${GL3W_PATH}/GL/*.c"
    "${GL3W_PATH}/GL/*.h"
)
source_group("gl3w" FILES ${GL3W_SOURCES})

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
	${SDL2_INCLUDE}
	${GL3W_PATH}
    ${IMGUI_PATH}
    ${IMGUI_IMPL_PATH}
)

link_directories( example_sdl_opengl3 ${SDL2_LIBRARY} )

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)

add_executable( example_sdl_opengl3
				${EXAMPLE_SDL_OPENGL3_HEADERS}
				${EXAMPLE_SDL_OPENGL3_SOURCES}
				${IMGUI_SOURCES}
				${IMGUI_IMPL_SOURCES}
				${GL3W_SOURCES}
)

target_link_libraries( example_sdl_opengl3 ${OPENGL_LIBRARIES} "-framework CoreFoundation" )
target_link_libraries( example_sdl_opengl3 general ${SDL2_LIBRARY} )

set_target_properties( example_sdl_opengl3 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
