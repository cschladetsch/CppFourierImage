#include <gtest/gtest.h>
#include "FourierTransform.hpp"
#include "ComplexImage.hpp"
#include <vector>
#include <complex>
#include <cmath>

class FourierTransformTest : public ::testing::Test {
protected:
    ComplexImage createTestImage() {
        // Create a simple test image with a pattern
        size_t width = 100;
        size_t height = 100;
        ComplexImage image(width, height);
        
        // Create a simple checkerboard pattern
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                double value = ((x / 10 + y / 10) % 2) ? 1.0 : 0.0;
                image.at(x, y) = std::complex<double>(value, 0.0);
            }
        }
        
        return image;
    }
};

TEST_F(FourierTransformTest, TransformCreatesFrequencies) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    auto indices = ft.getTopFrequencyIndices(transformed, 100);
    EXPECT_GT(indices.size(), 0);
}

TEST_F(FourierTransformTest, FrequencyAmplitudesAreNonNegative) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    // Check all magnitudes are non-negative
    for (size_t y = 0; y < transformed.getHeight(); ++y) {
        for (size_t x = 0; x < transformed.getWidth(); ++x) {
            EXPECT_GE(std::abs(transformed.at(x, y)), 0.0);
        }
    }
}

TEST_F(FourierTransformTest, TopFrequenciesAreOrdered) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    auto indices = ft.getTopFrequencyIndices(transformed, 10);
    
    // Check that top frequencies are properly selected
    EXPECT_LE(indices.size(), 10u);
    for (size_t i = 1; i < indices.size(); ++i) {
        auto mag_prev = std::abs(transformed.at(indices[i-1].first, indices[i-1].second));
        auto mag_curr = std::abs(transformed.at(indices[i].first, indices[i].second));
        EXPECT_GE(mag_prev, mag_curr);
    }
}

TEST_F(FourierTransformTest, DCComponentIsLargest) {
    // Create a constant image - should have strong DC component
    ComplexImage constantImage(50, 50);
    for (size_t i = 0; i < 50; ++i) {
        for (size_t j = 0; j < 50; ++j) {
            constantImage.at(i, j) = std::complex<double>(128.0, 0.0);
        }
    }
    
    FourierTransform ft;
    auto transformed = ft.transform2D(constantImage);
    transformed.fftShift(); // Center DC component
    
    // DC component should be at center after fftShift
    size_t cx = transformed.getWidth() / 2;
    size_t cy = transformed.getHeight() / 2;
    auto dc_magnitude = std::abs(transformed.at(cx, cy));
    
    // Check DC is larger than other components
    for (size_t y = 0; y < transformed.getHeight(); ++y) {
        for (size_t x = 0; x < transformed.getWidth(); ++x) {
            if (x != cx || y != cy) {
                EXPECT_GE(dc_magnitude, std::abs(transformed.at(x, y)));
            }
        }
    }
}

TEST_F(FourierTransformTest, HandlesEmptyImage) {
    ComplexImage emptyImage(0, 0);
    FourierTransform ft;
    
    auto transformed = ft.transform2D(emptyImage);
    
    EXPECT_EQ(transformed.getWidth(), 0u);
    EXPECT_EQ(transformed.getHeight(), 0u);
}

TEST_F(FourierTransformTest, PreservesImageDimensions) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    // Dimensions should remain the same after transform
    EXPECT_EQ(transformed.getWidth(), testImage.getWidth());
    EXPECT_EQ(transformed.getHeight(), testImage.getHeight());
}

TEST_F(FourierTransformTest, InverseTransformReconstructs) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    // Inverse transform should reconstruct the original
    auto reconstructed = ft.transform2D(transformed, FourierTransform::Direction::Inverse);
    
    // Compare with original (should be very close)
    for (size_t i = 0; i < testImage.getWidth(); ++i) {
        for (size_t j = 0; j < testImage.getHeight(); ++j) {
            EXPECT_NEAR(reconstructed.at(i, j).real(), 
                       testImage.at(i, j).real(), 1e-6);
        }
    }
}

TEST_F(FourierTransformTest, FrequencyMaskingWorks) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    // Apply low-pass filter
    auto filtered = ft.applyFrequencyMaskCircular(transformed, 0.1);
    
    // Verify dimensions are preserved
    EXPECT_EQ(filtered.getWidth(), transformed.getWidth());
    EXPECT_EQ(filtered.getHeight(), transformed.getHeight());
}

TEST_F(FourierTransformTest, KeepTopFrequenciesWorks) {
    ComplexImage testImage = createTestImage();
    FourierTransform ft;
    auto transformed = ft.transform2D(testImage);
    
    // Keep only top 10 frequencies
    auto filtered = ft.keepTopFrequencies(transformed, 10);
    
    // Count non-zero frequencies
    int nonZeroCount = 0;
    for (size_t y = 0; y < filtered.getHeight(); ++y) {
        for (size_t x = 0; x < filtered.getWidth(); ++x) {
            if (std::abs(filtered.at(x, y)) > 1e-10) {
                nonZeroCount++;
            }
        }
    }
    
    // Should have at most 10 non-zero frequencies
    EXPECT_LE(nonZeroCount, 10);
}