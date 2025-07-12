set(TEST_SOURCES
    test_main.cpp
    test_fourier_transform.cpp
    test_image_processor.cpp
    test_complex_image.cpp
    test_image_loader.cpp
    test_fourier_visualizer.cpp
)

# Source files to test (excluding main.cpp and UI)
set(SOURCE_FILES_TO_TEST
    ${CMAKE_SOURCE_DIR}/src/fourier_transform.cpp
    ${CMAKE_SOURCE_DIR}/src/image_processor.cpp
    ${CMAKE_SOURCE_DIR}/src/complex_image.cpp
    ${CMAKE_SOURCE_DIR}/src/image_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/fourier_visualizer.cpp
)

add_executable(fourier_tests ${TEST_SOURCES} ${SOURCE_FILES_TO_TEST})

# Include directories
target_include_directories(fourier_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${cimg_SOURCE_DIR}
    ${FFTW3_INCLUDE_DIR}
)

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