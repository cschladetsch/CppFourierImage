#include "ImageLoader.hpp"
#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"
#include <CImg.h>
#include <iostream>
#include <algorithm>
#include <ranges>

using namespace cimg_library;

ImageLoader::ImageLoader() {
    resetState();
}

ImageLoader::~ImageLoader() = default;

void ImageLoader::resetState() {
    complex_image_.reset();
    rgb_complex_image_.reset();
    rgb_data_.clear();
    width_ = 0;
    height_ = 0;
}

bool ImageLoader::loadImage(const std::string& filepath) {
    resetState();
    try {
        CImg<unsigned char> img(filepath.c_str());
        
        width_ = static_cast<size_t>(img.width());
        height_ = static_cast<size_t>(img.height());
        
        rgb_data_.resize(width_ * height_);

        auto image_coords = std::views::cartesian_product(
            std::views::iota(size_t{0}, height_),
            std::views::iota(size_t{0}, width_)
        );

        std::ranges::for_each(image_coords, [&](const auto& coord) {
            const auto [y, x] = coord;
            const int xi = static_cast<int>(x);
            const int yi = static_cast<int>(y);
            const uint8_t r = img(xi, yi, 0, 0);
            const uint8_t g = img.spectrum() > 1 ? img(xi, yi, 0, 1) : r;
            const uint8_t b = img.spectrum() > 2 ? img(xi, yi, 0, 2) : r;

            const uint32_t pixel = (static_cast<uint32_t>(r) << 24) |
                                   (static_cast<uint32_t>(g) << 16) |
                                   (static_cast<uint32_t>(b) << 8) |
                                   0xFF;

            rgb_data_[y * width_ + x] = pixel;
        });
        
        complex_image_ = std::make_shared<ComplexImage>(width_, height_);
        
        std::ranges::for_each(image_coords, [&](const auto& coord) {
            const auto [y, x] = coord;
            const uint32_t pixel = rgb_data_[y * width_ + x];
            const uint8_t r = static_cast<uint8_t>((pixel >> 24) & 0xFF);
            const uint8_t g = static_cast<uint8_t>((pixel >> 16) & 0xFF);
            const uint8_t b = static_cast<uint8_t>((pixel >> 8) & 0xFF);
            const float gray = 0.299f * r + 0.587f * g + 0.114f * b;
            const double value = gray / 255.0;
            complex_image_->at(x, y) = ComplexImage::Complex(value, 0.0);
        });
        
        rgb_complex_image_ = std::make_shared<RGBComplexImage>();
        rgb_complex_image_->setFromRGB(rgb_data_, width_, height_);
        
        std::cout << "Loaded image: " << width_ << "x" << height_ 
                  << " (" << img.spectrum() << " channels)" << std::endl;
        
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        resetState();
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        resetState();
        return false;
    }
}

std::shared_ptr<ComplexImage> ImageLoader::getComplexImage() const {
    return complex_image_;
}

std::shared_ptr<RGBComplexImage> ImageLoader::getRGBComplexImage() const {
    return rgb_complex_image_;
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
        const size_t width = static_cast<size_t>(image->getWidth());
        const size_t height = static_cast<size_t>(image->getHeight());
        CImg<unsigned char> img(static_cast<int>(width), static_cast<int>(height), 1, 1);
        
        const auto coords = std::views::cartesian_product(
            std::views::iota(size_t{0}, height),
            std::views::iota(size_t{0}, width)
        );

        double min_val = std::numeric_limits<double>::max();
        double max_val = std::numeric_limits<double>::lowest();

        std::ranges::for_each(coords, [&](const auto& coord) {
            const auto [y, x] = coord;
            const double mag = std::abs(image->at(x, y));
            min_val = std::min(min_val, mag);
            max_val = std::max(max_val, mag);
        });
        
        double range = max_val - min_val;
        if (range < 1e-6) range = 1.0;
        
        std::ranges::for_each(coords, [&](const auto& coord) {
            const auto [y, x] = coord;
            const double mag = std::abs(image->at(x, y));
            const double normalized = (mag - min_val) / range;
            img(static_cast<int>(x), static_cast<int>(y)) = static_cast<unsigned char>(normalized * 255);
        });
        
        img.save(filepath.c_str());
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error saving image: " << e.what() << std::endl;
        return false;
    }
}
