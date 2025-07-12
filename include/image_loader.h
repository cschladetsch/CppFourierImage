#pragma once

#include <memory>
#include <string>
#include <vector>

class ComplexImage;

class ImageLoader {
public:
    ImageLoader();
    ~ImageLoader();
    
    // Load an image from file and convert to ComplexImage
    bool loadImage(const std::string& filepath);
    
    // Get the loaded complex image
    std::shared_ptr<ComplexImage> getComplexImage() const;
    
    // Get list of supported image formats
    std::vector<std::string> getSupportedFormats() const;
    
    // Save complex image to file
    bool saveImage(const std::string& filepath, std::shared_ptr<ComplexImage> image) const;
    
private:
    std::shared_ptr<ComplexImage> m_complexImage;
};