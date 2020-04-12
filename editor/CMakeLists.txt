cmake_minimum_required(VERSION 3.15)
project(editor)

set(CMAKE_CXX_STANDARD 14)

include_directories(libs/imgui)
include_directories(libs/imgui/examples)
include_directories(libs/imgui/misc/cpp)
include_directories(libs/imgui/misc/freetype)
include_directories(libs/imgui/misc/single_file)
include_directories(/usr/include/SDL2) # Probably only works on Linux, TODO fix

add_executable(editor
        main.cpp
        libs/imgui/examples/imgui_impl_opengl3.cpp
        libs/imgui/examples/imgui_impl_opengl3.h
        libs/imgui/misc/cpp/imgui_stdlib.cpp
        libs/imgui/misc/cpp/imgui_stdlib.h
        libs/imgui/imconfig.h
        libs/imgui/imgui.cpp
        libs/imgui/imgui.h
        libs/imgui/imgui_demo.cpp
        libs/imgui/imgui_draw.cpp
        libs/imgui/imgui_internal.h
        libs/imgui/imgui_widgets.cpp
        libs/imgui/imstb_rectpack.h
        libs/imgui/imstb_textedit.h
        libs/imgui/imstb_truetype.h)

