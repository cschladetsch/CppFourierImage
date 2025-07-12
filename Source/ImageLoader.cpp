#include "ImageLoader.hpp"
#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"
#include <CImg.h>
#include <iostream>
#include <algorithm>

using namespace cimg_library;

ImageLoader::ImageLoader() : _complexImage(nullptr), _rgbComplexImage(nullptr) {
}

ImageLoader::~ImageLoader() {
}

bool ImageLoader::loadImage(const std::string& filepath) {
    try {
        // Load image using CImg - accepts any file type CImg supports
        CImg<unsigned char> img(filepath.c_str());
        
        _width = img.width();
        _height = img.height();
        
        // Prepare RGB data
        _rgbData.clear();
        _rgbData.resize(_width * _height);
        
        // Convert to RGB format
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                uint8_t r, g, b;
                
                // Get RGB values
                r = img(x, y, 0, 0);
                g = img.spectrum() > 1 ? img(x, y, 0, 1) : r;
                b = img.spectrum() > 2 ? img(x, y, 0, 2) : r;
                
                // Pack into 32-bit RGB format (0xFF in lowest byte for padding)
                uint32_t pixel = (static_cast<uint32_t>(r) << 24) |
                                (static_cast<uint32_t>(g) << 16) |
                                (static_cast<uint32_t>(b) << 8) |
                                0xFF;  // Padding byte
                
                _rgbData[y * _width + x] = pixel;
            }
        }
        
        // Create grayscale ComplexImage for Fourier processing
        _complexImage = std::make_shared<ComplexImage>(_width, _height);
        
        // Convert RGB to grayscale for ComplexImage
        for (size_t y = 0; y < _height; ++y) {
            for (size_t x = 0; x < _width; ++x) {
                uint32_t pixel = _rgbData[y * _width + x];
                uint8_t r = (pixel >> 24) & 0xFF;
                uint8_t g = (pixel >> 16) & 0xFF;
                uint8_t b = (pixel >> 8) & 0xFF;
                
                // Convert to grayscale using standard weights
                float gray = 0.299f * r + 0.587f * g + 0.114f * b;
                double value = gray / 255.0;
                _complexImage->at(x, y) = std::complex<double>(value, 0.0);
            }
        }
        
        // Create RGB complex image
        _rgbComplexImage = std::make_shared<RGBComplexImage>();
        _rgbComplexImage->setFromRGB(_rgbData, _width, _height);
        
        std::cout << "Loaded image: " << _width << "x" << _height 
                  << " (" << img.spectrum() << " channels)" << std::endl;
        
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        _complexImage = nullptr;
        _rgbComplexImage = nullptr;
        _rgbData.clear();
        _width = 0;
        _height = 0;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        _complexImage = nullptr;
        _rgbComplexImage = nullptr;
        _rgbData.clear();
        _width = 0;
        _height = 0;
        return false;
    }
}

std::shared_ptr<ComplexImage> ImageLoader::getComplexImage() const {
    return _complexImage;
}

std::shared_ptr<RGBComplexImage> ImageLoader::getRGBComplexImage() const {
    return _rgbComplexImage;
}

std::vector<std::string> ImageLoader::getSupportedFormats() const {
    // CImg supports many formats, return common ones
    std::vector<std::string> formats = {
        "bmp", "pgm", "ppm", "pnm",  // Always supported by CImg
        "png", "jpg", "jpeg", "gif",  // Common formats
        "tif", "tiff", "pbm", "hdr",  // Additional formats
        "exr", "ico", "cur"            // Extended formats
    };
    
    return formats;
}

bool ImageLoader::saveImage(const std::string& filepath, std::shared_ptr<ComplexImage> image) const {
    if (!image) {
        return false;
    }
    
    try {
        int width = image->getWidth();
        int height = image->getHeight();
        
        // Create CImg from complex image (using magnitude)
        CImg<unsigned char> img(width, height, 1, 1);
        
        // Find min/max for normalization
        double min_val = std::numeric_limits<double>::max();
        double max_val = std::numeric_limits<double>::min();
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double mag = std::abs(image->at(x, y));
                min_val = std::min(min_val, mag);
                max_val = std::max(max_val, mag);
            }
        }
        
        // Normalize and convert to 8-bit
        double range = max_val - min_val;
        if (range < 1e-6) range = 1.0;
        
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double mag = std::abs(image->at(x, y));
                double normalized = (mag - min_val) / range;
                img(x, y) = static_cast<unsigned char>(normalized * 255);
            }
        }
        
        img.save(filepath.c_str());
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error saving image: " << e.what() << std::endl;
        return false;
    }
}

