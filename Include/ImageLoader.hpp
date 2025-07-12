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
    const std::vector<uint32_t>& getRGBData() const { return _rgbData; }
    
    // Get image dimensions
    size_t getWidth() const { return _width; }
    size_t getHeight() const { return _height; }
    
    // Get list of supported image formats
    std::vector<std::string> getSupportedFormats() const;
    
    // Save complex image to file
    bool saveImage(const std::string& filepath, std::shared_ptr<ComplexImage> image) const;
    
private:
    std::shared_ptr<ComplexImage> _complexImage;  // For grayscale
    std::shared_ptr<RGBComplexImage> _rgbComplexImage;  // For RGB
    std::vector<uint32_t> _rgbData;  // 32-bit RGB format
    size_t _width = 0;
    size_t _height = 0;
    
};