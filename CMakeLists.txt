cmake_minimum_required(VERSION 3.1)
project(volcako LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup /NODEFAULTLIB:MSVCRT")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
    source/algorithm.cpp
    source/envelope.cpp
    source/envelope.h
    source/fileops.cpp
    source/fileops.h
    source/main.cpp
    source/operator.h
    source/settings.cpp
    source/synthstate.cpp
    source/synthstate.h
    source/sysex.cpp
    libs/imgui/imgui.cpp
    libs/imgui/imgui_demo.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_impl_glfw.cpp
    libs/imgui/imgui_impl_opengl3.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/imgui_widgets.cpp    
    libs/rtmidi/RtMidi.cpp
    libs/tinyfiledialogs/tinyfiledialogs.c
)

set_source_files_properties(
  libs/tinyfiledialogs/tinyfiledialogs.c
  PROPERTIES
  COMPILE_FLAGS /wd4267
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    libs
    libs/imgui
    libs/glfw/include
    libs/gl3w
)

target_link_directories(${PROJECT_NAME} PRIVATE "libs/glfw/lib-vc2010-64")
target_link_libraries(${PROJECT_NAME} PRIVATE glfw3 winmm opengl32)
