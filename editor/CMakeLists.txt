cmake_minimum_required(VERSION 3.15)
project(mwgeditor)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED COMPONENTS SDL2::SDL2)

include_directories(libs/imgui)
include_directories(libs/imgui/examples)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/misc/freetype)
include_directories(libs/imgui/misc/single_file)
include_directories(libs/stb)
include_directories(libs/json)
include_directories(libs/imfilebrowser)
include_directories(libs/imgui/examples/libs/gl3w)
include_directories(${SDL2_INCLUDE_DIRS})
#include_directories(/usr/include/SDL2) # Probably only works on Linux, TODO fix

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)

add_executable(mwgeditor
        src/main.cpp
        libs/imgui/examples/imgui_impl_opengl3.cpp
        libs/imgui/examples/libs/gl3w/GL/gl3w.c
        libs/imgui/examples/imgui_impl_opengl3.cpp
        libs/imgui/examples/imgui_impl_sdl.cpp
        libs/imgui/misc/cpp/imgui_stdlib.cpp
        libs/imgui/imgui.cpp
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imstb_truetype.h src/editor.cpp src/editor.h libs/stb/stb_image.h src/assetman.cpp src/assetman.h src/levelmodel.h src/loadjson.cpp src/loadjson.h src/savejson.cpp src/savejson.h src/util.cpp src/util.h libs/imfilebrowser/imfilebrowser.h src/visualizer.cpp src/visualizer.h src/global.cpp src/global.h src/vizmodel.h)

target_link_libraries(mwgeditor SDL2::SDL2)
