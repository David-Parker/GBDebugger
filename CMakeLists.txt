cmake_minimum_required(VERSION 3.9)

project(GBDebugger VERSION 0.0.1 DESCRIPTION "Emulator-agnostic GameBoy debugger using ImGui")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

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
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# GBDebugger source files
set(GBDEBUGGER_SOURCES
    src/GBDebugger.cpp
    src/DebuggerBackend.cpp
    src/panels/CPUStatePanel.cpp
    src/panels/FlagsPanel.cpp
    src/panels/MemoryViewerPanel.cpp
)

# GBDebugger library
add_library(GBDebugger STATIC
    ${GBDEBUGGER_SOURCES}
    ${IMGUI_SOURCES}
)

# Public include directories (API headers only)
target_include_directories(GBDebugger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Private include directories (ImGui and internal headers)
target_include_directories(GBDebugger PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# SDL2 include directories - support both standalone and parent project builds
if(TARGET SDL2)
    # SDL2 is built as part of parent project
    get_target_property(SDL2_INCLUDE_DIRS SDL2 INTERFACE_INCLUDE_DIRECTORIES)
    if(SDL2_INCLUDE_DIRS)
        target_include_directories(GBDebugger PRIVATE ${SDL2_INCLUDE_DIRS})
    endif()
    # Also add the SDL2 source include directory directly
    target_include_directories(GBDebugger PRIVATE ${CMAKE_SOURCE_DIR}/dep/SDL2/include)
else()
    # Standalone build - try to find SDL2
    find_package(SDL2 REQUIRED)
    target_include_directories(GBDebugger PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

# Link against OpenGL
target_link_libraries(GBDebugger PUBLIC OpenGL::GL)

# Link against SDL2 if available as target
if(TARGET SDL2)
    target_link_libraries(GBDebugger PUBLIC SDL2)
elseif(SDL2_LIBRARIES)
    target_link_libraries(GBDebugger PUBLIC ${SDL2_LIBRARIES})
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(GBDebugger PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# macOS-specific OpenGL framework
if(APPLE)
    target_link_libraries(GBDebugger PUBLIC "-framework OpenGL")
endif()

# Optional: Build tests
option(BUILD_GBDEBUGGER_TESTS "Build GBDebugger tests" OFF)

if(BUILD_GBDEBUGGER_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Optional: Build example
option(BUILD_GBDEBUGGER_EXAMPLE "Build GBDebugger example" OFF)

if(BUILD_GBDEBUGGER_EXAMPLE)
    add_executable(GBDebuggerExample
        example/example.cpp
    )
    
    target_link_libraries(GBDebuggerExample GBDebugger)
    
    target_include_directories(GBDebuggerExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
endif()
