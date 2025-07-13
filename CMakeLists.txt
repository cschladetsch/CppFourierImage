cmake_minimum_required(VERSION 3.20)
project(FourierImageViewer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/Bin)

# Add FetchContent for dependencies
include(FetchContent)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(TBB REQUIRED)

# Find FFTW3 for Fourier transforms
find_library(FFTW3_LIBRARY fftw3)
find_path(FFTW3_INCLUDE_DIR fftw3.h)
if(NOT FFTW3_LIBRARY OR NOT FFTW3_INCLUDE_DIR)
    message(WARNING "FFTW3 not found. Install libfftw3-dev")
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

FetchContent_MakeAvailable(glfw imgui cimg)

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
if(FFTW3_INCLUDE_DIR)
    include_directories(${FFTW3_INCLUDE_DIR})
endif()
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
target_link_libraries(fourier_viewer 
    glfw
    glad
    ${OPENGL_LIBRARIES}
    ${FFTW3_LIBRARY}
    Threads::Threads
    TBB::tbb
    m
    ${CMAKE_DL_LIBS}
)
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