#include <gtest/gtest.h>
#include "image_processor.h"
#include "complex_image.h"
#include <vector>
#include <algorithm>

class ImageProcessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test image
        width = 10;
        height = 10;
        testImage = ComplexImage(width, height);
        
        // Fill with gradient values
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                double value = static_cast<double>((y * width + x) % 256) / 255.0;
                testImage.at(x, y) = std::complex<double>(value, 0.0);
            }
        }
    }
    
    ComplexImage testImage;
    size_t width, height;
};

TEST_F(ImageProcessorTest, PadToPowerOfTwo) {
    auto padded = ImageProcessor::padToPowerOfTwo(testImage);
    
    // Check that dimensions are powers of two
    size_t padded_width = padded.getWidth();
    size_t padded_height = padded.getHeight();
    
    EXPECT_TRUE((padded_width & (padded_width - 1)) == 0);
    EXPECT_TRUE((padded_height & (padded_height - 1)) == 0);
    EXPECT_GE(padded_width, width);
    EXPECT_GE(padded_height, height);
}

TEST_F(ImageProcessorTest, CropToOriginalSize) {
    auto padded = ImageProcessor::padToPowerOfTwo(testImage);
    auto cropped = ImageProcessor::cropToOriginalSize(padded, width, height);
    
    EXPECT_EQ(cropped.getWidth(), width);
    EXPECT_EQ(cropped.getHeight(), height);
}

TEST_F(ImageProcessorTest, NormalizeToUint8) {
    std::vector<double> data;
    for (int i = 0; i <= 255; ++i) {
        data.push_back(i);
    }
    
    auto normalized = ImageProcessor::normalizeToUint8(data);
    
    EXPECT_EQ(normalized.size(), data.size());
    EXPECT_EQ(normalized[0], 0);
    EXPECT_EQ(normalized[255], 255);
}

TEST_F(ImageProcessorTest, NormalizeToFloat) {
    std::vector<double> data{0.0, 128.0, 255.0};
    
    auto normalized = ImageProcessor::normalizeToFloat(data);
    
    EXPECT_EQ(normalized.size(), data.size());
    EXPECT_FLOAT_EQ(normalized[0], 0.0f);
    EXPECT_FLOAT_EQ(normalized[2], 1.0f);
}

TEST_F(ImageProcessorTest, ApplyGaussianBlur) {
    auto blurred = ImageProcessor::applyGaussianBlur(testImage, 1.0);
    
    EXPECT_EQ(blurred.getWidth(), testImage.getWidth());
    EXPECT_EQ(blurred.getHeight(), testImage.getHeight());
    
    // Blurred image should have reduced high-frequency content
    // Center pixels should be affected by blur
    auto centerOriginal = testImage.at(5, 5);
    auto centerBlurred = blurred.at(5, 5);
    
    // The blurred value should be different from original (unless it's surrounded by identical values)
    EXPECT_NE(centerBlurred.real(), centerOriginal.real());
}

TEST_F(ImageProcessorTest, BlurWithZeroSigma) {
    auto blurred = ImageProcessor::applyGaussianBlur(testImage, 0.0);
    
    // With sigma = 0, image should be unchanged
    for (size_t y = 0; y < testImage.getHeight(); ++y) {
        for (size_t x = 0; x < testImage.getWidth(); ++x) {
            EXPECT_DOUBLE_EQ(blurred.at(x, y).real(), testImage.at(x, y).real());
            EXPECT_DOUBLE_EQ(blurred.at(x, y).imag(), testImage.at(x, y).imag());
        }
    }
}

TEST_F(ImageProcessorTest, ApplyEdgeDetection) {
    // Create an image with clear edges
    ComplexImage edgeImage(10, 10);
    for (size_t y = 0; y < 5; ++y) {
        for (size_t x = 0; x < 10; ++x) {
            edgeImage.at(x, y) = std::complex<double>(1.0, 0.0);
        }
    }
    for (size_t y = 5; y < 10; ++y) {
        for (size_t x = 0; x < 10; ++x) {
            edgeImage.at(x, y) = std::complex<double>(0.0, 0.0);
        }
    }
    
    auto edges = ImageProcessor::applyEdgeDetection(edgeImage);
    
    EXPECT_EQ(edges.getWidth(), edgeImage.getWidth());
    EXPECT_EQ(edges.getHeight(), edgeImage.getHeight());
    
    // There should be high values at the horizontal edge
    for (size_t x = 1; x < 9; ++x) {
        EXPECT_GT(std::abs(edges.at(x, 5)), 0.1);
    }
}

TEST_F(ImageProcessorTest, ApplyLogScale) {
    std::vector<double> magnitude_data{1.0, 10.0, 100.0, 1000.0};
    auto original_data = magnitude_data;
    
    ImageProcessor::applyLogScale(magnitude_data);
    
    EXPECT_EQ(magnitude_data.size(), original_data.size());
    
    // Log scale should preserve order
    for (size_t i = 1; i < magnitude_data.size(); ++i) {
        EXPECT_GT(magnitude_data[i], magnitude_data[i-1]);
    }
}

TEST_F(ImageProcessorTest, ApplyColorMap) {
    std::vector<uint8_t> grayscale{0, 64, 128, 192, 255};
    std::vector<uint8_t> rgb_output;
    
    ImageProcessor::applyColorMap(grayscale, rgb_output);
    
    // Should have 3 channels (RGB) for each grayscale pixel
    EXPECT_EQ(rgb_output.size(), grayscale.size() * 3);
    
    // Check that black maps to black and white maps to white
    EXPECT_EQ(rgb_output[0], 0);     // R for black
    EXPECT_EQ(rgb_output[1], 0);     // G for black
    EXPECT_EQ(rgb_output[2], 0);     // B for black
    
    EXPECT_EQ(rgb_output[12], 255);  // R for white
    EXPECT_EQ(rgb_output[13], 255);  // G for white
    EXPECT_EQ(rgb_output[14], 255);  // B for white
}