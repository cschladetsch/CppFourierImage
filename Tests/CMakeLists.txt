set(TEST_SOURCES
    TestMain.cpp
    TestFourierTransform.cpp
    TestImageProcessor.cpp
    TestComplexImage.cpp
    TestImageLoader.cpp
    TestFourierVisualizer.cpp
)

# Source files to test (excluding main.cpp and UI)
set(SOURCE_FILES_TO_TEST
    ${CMAKE_SOURCE_DIR}/Source/FourierTransform.cpp
    ${CMAKE_SOURCE_DIR}/Source/ImageProcessor.cpp
    ${CMAKE_SOURCE_DIR}/Source/ComplexImage.cpp
    ${CMAKE_SOURCE_DIR}/Source/ImageLoader.cpp
    ${CMAKE_SOURCE_DIR}/Source/FourierVisualizer.cpp
    ${CMAKE_SOURCE_DIR}/Source/RgbComplexImage.cpp
)

add_executable(fourier_tests ${TEST_SOURCES} ${SOURCE_FILES_TO_TEST})

# Include directories
target_include_directories(fourier_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/Include
    ${cimg_SOURCE_DIR}
)
if(FFTW3_INCLUDE_DIR)
    target_include_directories(fourier_tests PRIVATE ${FFTW3_INCLUDE_DIR})
endif()

# Link libraries
target_link_libraries(fourier_tests 
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
    m
)

if(FFTW3_LIBRARY)
    target_link_libraries(fourier_tests ${FFTW3_LIBRARY})
endif()

if(PNG_FOUND)
    target_link_libraries(fourier_tests ${PNG_LIBRARIES})
endif()
if(JPEG_FOUND)
    target_link_libraries(fourier_tests ${JPEG_LIBRARIES})
endif()

# CImg definitions for tests
target_compile_definitions(fourier_tests PRIVATE cimg_display=0)
if(PNG_FOUND)
    target_compile_definitions(fourier_tests PRIVATE cimg_use_png)
endif()
if(JPEG_FOUND)
    target_compile_definitions(fourier_tests PRIVATE cimg_use_jpeg)
endif()

include(GoogleTest)
gtest_discover_tests(fourier_tests)
