#pragma once

#include "ComplexImage.hpp"
#include "Types.hpp"
#include <array>
#include <memory>

class RGBComplexImage {
public:
    RGBComplexImage() = default;
    RGBComplexImage(Scalar width, Scalar height);
    
    // Get individual channel
    ComplexImage& getChannel(int channel) { return channels_[channel]; }
    const ComplexImage& getChannel(int channel) const { return channels_[channel]; }
    
    // Get dimensions
    Scalar getWidth() const { return width_; }
    Scalar getHeight() const { return height_; }
    
    // Create from RGB data
    void setFromRGB(const std::vector<uint32_t>& rgbData, Scalar width, Scalar height);
    
    // Convert back to RGB
    std::vector<uint32_t> toRGB() const;
    
    // Get magnitude images for each channel
    std::array<std::vector<Scalar>, 3> getMagnitudeImages() const;
    
private:
    std::array<ComplexImage, 3> channels_; // R, G, B channels
    Scalar width_ = 0.0;
    Scalar height_ = 0.0;
};