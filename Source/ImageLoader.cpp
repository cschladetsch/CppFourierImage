#include <CImg.h>
#include <iostream>
#include <algorithm>
#include <ranges>
#include <vector>
#include <cstdint>

// stb_image decodes JPEG, PNG, BMP, GIF, TGA, PSD, HDR, PIC and PNM itself
#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

#include "ImageLoader.hpp"
#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"

using namespace cimg_library;

namespace {

// An image decoded to 8-bit RGB, three bytes per pixel, row-major
struct DecodedImage {
    size_t width = 0;
    size_t height = 0;
    int source_channels = 0;
    std::vector<uint8_t> rgb;
};

// Decode with stb_image. Returns false if stb_image cannot read the file.
bool decodeWithStb(const std::string& filepath, DecodedImage& out) {
    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
    if (!pixels) {
        return false;
    }

    out.width = static_cast<size_t>(width);
    out.height = static_cast<size_t>(height);
    out.source_channels = channels;
    out.rgb.assign(pixels, pixels + out.width * out.height * 3);
    stbi_image_free(pixels);
    return true;
}

// Decode with CImg, for formats stb_image does not handle (e.g. TIFF via an
// external converter). Throws CImgException on failure.
DecodedImage decodeWithCImg(const std::string& filepath) {
    CImg<unsigned char> img(filepath.c_str());

    DecodedImage out;
    out.width = static_cast<size_t>(img.width());
    out.height = static_cast<size_t>(img.height());
    out.source_channels = img.spectrum();
    out.rgb.resize(out.width * out.height * 3);

    for (size_t y = 0; y < out.height; ++y) {
        for (size_t x = 0; x < out.width; ++x) {
            const int xi = static_cast<int>(x);
            const int yi = static_cast<int>(y);
            const uint8_t r = img(xi, yi, 0, 0);
            const uint8_t g = img.spectrum() > 1 ? img(xi, yi, 0, 1) : r;
            const uint8_t b = img.spectrum() > 2 ? img(xi, yi, 0, 2) : r;
            uint8_t* p = &out.rgb[(y * out.width + x) * 3];
            p[0] = r;
            p[1] = g;
            p[2] = b;
        }
    }
    return out;
}

} // namespace

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
        DecodedImage decoded;
        if (!decodeWithStb(filepath, decoded)) {
            decoded = decodeWithCImg(filepath);
        }

        width_ = decoded.width;
        height_ = decoded.height;

        rgb_data_.resize(width_ * height_);

        auto image_coords = std::views::cartesian_product(
            std::views::iota(size_t{0}, height_),
            std::views::iota(size_t{0}, width_)
        );

        std::ranges::for_each(image_coords, [&](const auto& coord) {
            const auto [y, x] = coord;
            const uint8_t* p = &decoded.rgb[(y * width_ + x) * 3];
            const uint8_t r = p[0];
            const uint8_t g = p[1];
            const uint8_t b = p[2];

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
                  << " (" << decoded.source_channels << " channels)" << std::endl;
        
        return true;
        
    } catch (const CImgException& e) {
        std::cerr << "Error loading image: " << filepath
                  << " (stb_image: " << stbi_failure_reason() << "; CImg: " << e.what() << ")" << std::endl;
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
    std::vector<std::string> formats = {
        // Decoded by the bundled stb_image
        "png", "jpg", "jpeg", "bmp", "gif", "tga", "psd", "hdr", "pic",
        "pgm", "ppm", "pnm",
        // Passed to CImg, which may need an external converter (e.g. ImageMagick)
        "tif", "tiff", "pbm", "exr", "ico", "cur"
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
