cmake_minimum_required(VERSION 3.20)
project(FourierImageViewer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Visual Studio: stop vcpkg's user-wide MSBuild integration ("vcpkg integrate install")
# from adding its headers and libraries to these projects. Dependencies come from
# FetchContent, and mixing in vcpkg's copies (e.g. its gmock.dll) crashes the tests.
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false")

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Bin)

# Add FetchContent for dependencies
include(FetchContent)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
# Intel TBB is only used as the std::execution backend for libstdc++.
# MSVC's standard library does not need it; without it libstdc++ runs the
# parallel algorithms sequentially.
find_package(TBB QUIET)
if(NOT TBB_FOUND AND NOT MSVC)
    message(WARNING "TBB not found: std::execution algorithms will run sequentially")
endif()

# Find image libraries for CImg
find_package(PNG)
find_package(JPEG)

# Fetch GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
    GIT_SHALLOW TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED TRUE
)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
    GIT_SHALLOW TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED TRUE
)

# Fetch CImg
FetchContent_Declare(
    cimg
    GIT_REPOSITORY https://github.com/dtschump/CImg.git
    GIT_TAG v.3.3.3
    GIT_SHALLOW TRUE
    FETCHCONTENT_UPDATES_DISCONNECTED TRUE
)

# Only the GLFW library is needed, not its examples, tests or docs
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw imgui cimg)

# stb_image (single public-domain header) decodes JPEG, PNG, BMP, GIF, etc.
# so no system image libraries are needed. Pinned to a commit and checked by hash.
set(STB_COMMIT 6b5799bae8a1fe3792691ef4b72ae26434c3e799)
set(STB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/_deps/stb/include)
if(NOT EXISTS ${STB_INCLUDE_DIR}/stb_image.h)
    file(DOWNLOAD
        https://raw.githubusercontent.com/nothings/stb/${STB_COMMIT}/stb_image.h
        ${STB_INCLUDE_DIR}/stb_image.h
        EXPECTED_HASH SHA256=dfc2aadda6295f67f677587773f25e5bf8c8ad427ed0db7c518525c91e72e020
        TLS_VERIFY ON
    )
endif()
include_directories(SYSTEM ${STB_INCLUDE_DIR})

# Use local GLAD files
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/External)
add_library(glad STATIC ${GLAD_DIR}/glad/glad.c)
target_include_directories(glad PUBLIC ${GLAD_DIR})

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# CImg configuration
add_definitions(-Dcimg_display=0) # Disable CImg display to avoid X11 dependency on WSL
if(PNG_FOUND)
    add_definitions(-Dcimg_use_png)
endif()
if(JPEG_FOUND)
    add_definitions(-Dcimg_use_jpeg)
endif()

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIRS}
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/cpp
    ${cimg_SOURCE_DIR}
    Include
)
if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
endif()
if(JPEG_FOUND)
    include_directories(${JPEG_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    Source/Main.cpp
    Source/FourierTransform.cpp
    Source/ImageProcessor.cpp
    Source/Renderer.cpp
    Source/UiManager.cpp
    Source/FourierVisualizer.cpp
    Source/ImageLoader.cpp
    Source/ComplexImage.cpp
    Source/RgbComplexImage.cpp
)

# Header files
set(HEADERS
    Include/FourierTransform.hpp
    Include/ImageProcessor.hpp
    Include/Renderer.hpp
    Include/UiManager.hpp
    Include/ComplexImage.hpp
    Include/FourierVisualizer.hpp
    Include/ImageLoader.hpp
    Include/RgbComplexImage.hpp
)

# Main executable
add_executable(fourier_viewer ${SOURCES} ${HEADERS} ${IMGUI_SOURCES})
# glad provides the GL declarations, so GLFW must not include the system GL header
target_compile_definitions(fourier_viewer PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(fourier_viewer 
    glfw
    glad
    ${OPENGL_LIBRARIES}
    Threads::Threads
    ${CMAKE_DL_LIBS}
)
if(TBB_FOUND)
    target_link_libraries(fourier_viewer TBB::tbb)
endif()
if(UNIX)
    target_link_libraries(fourier_viewer m)
endif()
if(PNG_FOUND)
    target_link_libraries(fourier_viewer ${PNG_LIBRARIES})
endif()
if(JPEG_FOUND)
    target_link_libraries(fourier_viewer ${JPEG_LIBRARIES})
endif()

# Platform-specific settings
if(WIN32)
    target_link_libraries(fourier_viewer opengl32)
elseif(APPLE)
    target_link_libraries(fourier_viewer "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
elseif(UNIX)
    # For Linux/WSL2
    find_package(X11)
    if(X11_FOUND)
        target_link_libraries(fourier_viewer ${X11_LIBRARIES})
    endif()
endif()

# Enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(fourier_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Copy resources if they exist
if(EXISTS ${CMAKE_SOURCE_DIR}/resources)
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Testing (optional)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    
    # Download and configure GoogleTest
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(Tests)
endif()