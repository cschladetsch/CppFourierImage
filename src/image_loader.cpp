#include "image_loader.h"
#include "complex_image.h"
#include <CImg.h>
#include <iostream>
#include <algorithm>

using namespace cimg_library;

ImageLoader::ImageLoader() : m_complexImage(nullptr) {
}

ImageLoader::~ImageLoader() {
}

bool ImageLoader::loadImage(const std::string& filepath) {
    try {
        // Load image using CImg
        CImg<unsigned char> img(filepath.c_str());
        
        // Convert to grayscale if needed
        if (img.spectrum() > 1) {
            // Convert RGB to grayscale using standard weights
            CImg<unsigned char> gray(img.width(), img.height(), 1, 1);
            for (int y = 0; y < img.height(); ++y) {
                for (int x = 0; x < img.width(); ++x) {
                    float r = img(x, y, 0, 0);
                    float g = img.spectrum() > 1 ? img(x, y, 0, 1) : r;
                    float b = img.spectrum() > 2 ? img(x, y, 0, 2) : r;
                    gray(x, y) = static_cast<unsigned char>(0.299f * r + 0.587f * g + 0.114f * b);
                }
            }
            img = gray;
        }
        
        // Create ComplexImage from grayscale data
        m_complexImage = std::make_shared<ComplexImage>(img.width(), img.height());
        
        // Copy pixel data, normalizing to [0, 1]
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                double value = static_cast<double>(img(x, y)) / 255.0;
                m_complexImage->at(x, y) = std::complex<double>(value, 0.0);
            }
        }
        
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        m_complexImage = nullptr;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        m_complexImage = nullptr;
        return false;
    }
}

std::shared_ptr<ComplexImage> ImageLoader::getComplexImage() const {
    return m_complexImage;
}

std::vector<std::string> ImageLoader::getSupportedFormats() const {
    std::vector<std::string> formats = {
        "bmp", "pgm", "ppm", "pnm"  // Always supported by CImg
    };
    
    #ifdef cimg_use_png
    formats.push_back("png");
    #endif
    
    #ifdef cimg_use_jpeg
    formats.push_back("jpg");
    formats.push_back("jpeg");
    #endif
    
    #ifdef cimg_use_tiff
    formats.push_back("tif");
    formats.push_back("tiff");
    #endif
    
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