cmake_minimum_required(VERSION 3.20)
project(FourierImageViewer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add FetchContent for dependencies
include(FetchContent)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

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
)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
)

# Fetch CImg
FetchContent_Declare(
    cimg
    GIT_REPOSITORY https://github.com/dtschump/CImg.git
    GIT_TAG v.3.3.3
)

FetchContent_MakeAvailable(glfw imgui cimg)

# Use local GLAD files
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/external)
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
    ${FFTW3_INCLUDE_DIR}
    include
)
if(PNG_FOUND)
    include_directories(${PNG_INCLUDE_DIRS})
endif()
if(JPEG_FOUND)
    include_directories(${JPEG_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/fourier_transform.cpp
    src/image_processor.cpp
    src/renderer.cpp
    src/ui_manager.cpp
    src/fourier_visualizer.cpp
    src/image_loader.cpp
    src/complex_image.cpp
)

# Header files
set(HEADERS
    include/fourier_transform.h
    include/image_processor.h
    include/renderer.h
    include/ui_manager.h
    include/complex_image.h
    include/fourier_visualizer.h
    include/image_loader.h
)

# Main executable
add_executable(fourier_viewer ${SOURCES} ${HEADERS} ${IMGUI_SOURCES})
target_link_libraries(fourier_viewer 
    glfw
    glad
    ${OPENGL_LIBRARIES}
    ${FFTW3_LIBRARY}
    Threads::Threads
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
    
    add_subdirectory(tests)
endif()