#include "RgbComplexImage.hpp"
#include <algorithm>

RGBComplexImage::RGBComplexImage(Scalar width, Scalar height) 
    : width_(width), height_(height) {
    for (auto& channel : channels_) {
        channel.resize(width, height);
    }
}

void RGBComplexImage::setFromRGB(const std::vector<uint32_t>& rgbData, Scalar width, Scalar height) {
    width_ = width;
    height_ = height;
    
    // Resize channels
    for (auto& channel : channels_) {
        channel.resize(width, height);
    }
    
    // Extract RGB channels
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            uint32_t pixel = rgbData[y * width + x];
            uint8_t r = (pixel >> 24) & 0xFF;
            uint8_t g = (pixel >> 16) & 0xFF;
            uint8_t b = (pixel >> 8) & 0xFF;
            
            // Normalize to [0, 1] and store in complex form
            channels_[0].at(x, y) = Complex(r / 255.0, 0.0);
            channels_[1].at(x, y) = Complex(g / 255.0, 0.0);
            channels_[2].at(x, y) = Complex(b / 255.0, 0.0);
        }
    }
}

std::vector<uint32_t> RGBComplexImage::toRGB() const {
    std::vector<uint32_t> rgbData(width_ * height_);
    
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            // Get real parts and clamp to [0, 1]
            Scalar r = std::clamp(channels_[0].at(x, y).real(), Scalar(0.0), Scalar(1.0));
            Scalar g = std::clamp(channels_[1].at(x, y).real(), Scalar(0.0), Scalar(1.0));
            Scalar b = std::clamp(channels_[2].at(x, y).real(), Scalar(0.0), Scalar(1.0));
            
            // Convert to 8-bit and pack
            uint8_t r8 = static_cast<uint8_t>(r * 255);
            uint8_t g8 = static_cast<uint8_t>(g * 255);
            uint8_t b8 = static_cast<uint8_t>(b * 255);
            
            rgbData[y * width_ + x] = (static_cast<uint32_t>(r8) << 24) |
                                      (static_cast<uint32_t>(g8) << 16) |
                                      (static_cast<uint32_t>(b8) << 8) |
                                      0xFF;
        }
    }
    
    return rgbData;
}

std::array<std::vector<Scalar>, 3> RGBComplexImage::getMagnitudeImages() const {
    std::array<std::vector<Scalar>, 3> magnitudes;
    
    for (int i = 0; i < 3; ++i) {
        magnitudes[i] = channels_[i].getMagnitudeImage();
    }
    
    return magnitudes;
}