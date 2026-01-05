cmake_minimum_required(VERSION 3.9)

project(GBDebugger VERSION 0.0.1 DESCRIPTION "Emulator-agnostic GameBoy debugger using ImGui")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_DIR ${imgui_SOURCE_DIR})

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# GBDebugger library
add_library(GBDebugger STATIC
    src/GBDebugger.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(GBDebugger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(GBDebugger PRIVATE
    ${IMGUI_DIR}
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(GBDebugger PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Optional: Build tests
option(BUILD_GBDEBUGGER_TESTS "Build GBDebugger tests" OFF)

if(BUILD_GBDEBUGGER_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Optional: Build example
option(BUILD_GBDEBUGGER_EXAMPLE "Build GBDebugger example" ON)

if(BUILD_GBDEBUGGER_EXAMPLE)
    add_executable(GBDebuggerExample
        example/example.cpp
    )
    
    target_link_libraries(GBDebuggerExample GBDebugger)
    
    target_include_directories(GBDebuggerExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()
