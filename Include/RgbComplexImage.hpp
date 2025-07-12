#pragma once

#include "ComplexImage.hpp"
#include <array>
#include <memory>

class RGBComplexImage {
public:
    RGBComplexImage() = default;
    RGBComplexImage(size_t width, size_t height);
    
    // Get individual channel
    ComplexImage& getChannel(int channel) { return channels_[channel]; }
    const ComplexImage& getChannel(int channel) const { return channels_[channel]; }
    
    // Get dimensions
    size_t getWidth() const { return width_; }
    size_t getHeight() const { return height_; }
    
    // Create from RGB data
    void setFromRGB(const std::vector<uint32_t>& rgbData, size_t width, size_t height);
    
    // Convert back to RGB
    std::vector<uint32_t> toRGB() const;
    
    // Get magnitude images for each channel
    std::array<std::vector<double>, 3> getMagnitudeImages() const;
    
private:
    std::array<ComplexImage, 3> channels_; // R, G, B channels
    size_t width_ = 0;
    size_t height_ = 0;
};