cmake_minimum_required(VERSION 3.16)
project(mwgeditor)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(lib/glfw-3.3.2)
find_package(OpenGL REQUIRED)
include_directories(src lib/imgui lib/imgui/examples lib/glad/include lib/json lib/stb lib/imfilebrowser)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

add_executable(mwgeditor
        src/main.cpp
        src/assetman.cpp
        src/editor.cpp
        src/global.cpp
        src/loadjson.cpp
        src/savejson.cpp
        src/util.cpp
        src/visualizer.cpp
        lib/imgui/imgui.cpp
        lib/imgui/imgui_draw.cpp
        lib/imgui/imgui_widgets.cpp
        lib/imgui/imgui_demo.cpp
        lib/imgui/examples/imgui_impl_opengl3.cpp
        lib/imgui/examples/imgui_impl_glfw.cpp
        lib/glad/src/glad.c src/recipeeditor.cpp src/recipeeditor.h)

target_link_libraries(mwgeditor glfw ${GLFW_LIBRARIES} ${OPENGL_LIBRARIES})
