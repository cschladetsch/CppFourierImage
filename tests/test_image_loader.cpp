#include <gtest/gtest.h>
#include "image_loader.h"
#include "complex_image.h"
#include <fstream>
#include <cstdio>

class ImageLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        loader = std::make_unique<ImageLoader>();
        
        // Create a temporary test image file (simple PGM format)
        createTestPGMFile("test_image.pgm", 4, 4);
    }
    
    void TearDown() override {
        // Clean up test files
        std::remove("test_image.pgm");
        std::remove("nonexistent.png");
    }
    
    void createTestPGMFile(const std::string& filename, int width, int height) {
        std::ofstream file(filename, std::ios::binary);
        file << "P5\n" << width << " " << height << "\n255\n";
        
        // Create a simple gradient pattern
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                unsigned char value = static_cast<unsigned char>((x + y) * 255 / (width + height - 2));
                file.write(reinterpret_cast<char*>(&value), 1);
            }
        }
        file.close();
    }
    
    std::unique_ptr<ImageLoader> loader;
};

TEST_F(ImageLoaderTest, LoadValidPGMFile) {
    EXPECT_TRUE(loader->loadImage("test_image.pgm"));
    
    auto complex_image = loader->getComplexImage();
    ASSERT_NE(complex_image, nullptr);
    EXPECT_EQ(complex_image->getWidth(), 4u);
    EXPECT_EQ(complex_image->getHeight(), 4u);
}

TEST_F(ImageLoaderTest, LoadNonExistentFile) {
    EXPECT_FALSE(loader->loadImage("nonexistent.png"));
    EXPECT_EQ(loader->getComplexImage(), nullptr);
}

TEST_F(ImageLoaderTest, LoadEmptyFilename) {
    EXPECT_FALSE(loader->loadImage(""));
    EXPECT_EQ(loader->getComplexImage(), nullptr);
}

TEST_F(ImageLoaderTest, GetComplexImageBeforeLoad) {
    auto complex_image = loader->getComplexImage();
    EXPECT_EQ(complex_image, nullptr);
}

TEST_F(ImageLoaderTest, ConvertToGrayscale) {
    EXPECT_TRUE(loader->loadImage("test_image.pgm"));
    
    auto complex_image = loader->getComplexImage();
    ASSERT_NE(complex_image, nullptr);
    
    // Check that all values are real (imaginary part is zero)
    for (size_t y = 0; y < complex_image->getHeight(); ++y) {
        for (size_t x = 0; x < complex_image->getWidth(); ++x) {
            auto pixel = complex_image->at(x, y);
            EXPECT_EQ(pixel.imag(), 0.0);
            EXPECT_GE(pixel.real(), 0.0);
            EXPECT_LE(pixel.real(), 1.0); // Normalized to [0, 1]
        }
    }
}

TEST_F(ImageLoaderTest, MultipleLoads) {
    // First load
    EXPECT_TRUE(loader->loadImage("test_image.pgm"));
    auto first_image = loader->getComplexImage();
    ASSERT_NE(first_image, nullptr);
    size_t first_width = first_image->getWidth();
    
    // Create a different sized image
    createTestPGMFile("test_image2.pgm", 8, 8);
    
    // Second load should replace the first
    EXPECT_TRUE(loader->loadImage("test_image2.pgm"));
    auto second_image = loader->getComplexImage();
    ASSERT_NE(second_image, nullptr);
    EXPECT_EQ(second_image->getWidth(), 8u);
    EXPECT_NE(second_image->getWidth(), first_width);
    
    // Clean up
    std::remove("test_image2.pgm");
}

TEST_F(ImageLoaderTest, SupportedFormats) {
    // Test that the loader reports support for common formats
    auto formats = loader->getSupportedFormats();
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "pgm") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "ppm") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "bmp") != formats.end());
    
    // PNG and JPEG support depends on compilation flags
    #ifdef cimg_use_png
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "png") != formats.end());
    #endif
    
    #ifdef cimg_use_jpeg
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "jpg") != formats.end());
    EXPECT_TRUE(std::find(formats.begin(), formats.end(), "jpeg") != formats.end());
    #endif
}