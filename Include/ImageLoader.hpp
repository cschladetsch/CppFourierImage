#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

class ComplexImage;
class RGBComplexImage;

class ImageLoader {
public:
    ImageLoader();
    ~ImageLoader();
    
    // Load an image from file and convert to ComplexImage
    bool loadImage(const std::string& filepath);
    
    // Get the loaded complex image (grayscale)
    std::shared_ptr<ComplexImage> getComplexImage() const;
    
    // Get the loaded RGB complex image
    std::shared_ptr<RGBComplexImage> getRGBComplexImage() const;
    
    // Get the original RGB data (32-bit RGB)
    const std::vector<uint32_t>& getRGBData() const { return rgb_data_; }
    
    // Get image dimensions
    size_t getWidth() const { return width_; }
    size_t getHeight() const { return height_; }
    
    // Get list of supported image formats
    std::vector<std::string> getSupportedFormats() const;
    
    // Save complex image to file
    bool saveImage(const std::string& filepath, std::shared_ptr<ComplexImage> image) const;
    
private:
    void resetState();

    std::shared_ptr<ComplexImage> complex_image_;  // For grayscale
    std::shared_ptr<RGBComplexImage> rgb_complex_image_;  // For RGB
    std::vector<uint32_t> rgb_data_;  // 32-bit RGB format
    size_t width_ = 0;
    size_t height_ = 0;
    
};
