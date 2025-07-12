#include "ComplexImage.hpp"
#include <algorithm>
#include <cmath>

ComplexImage::ComplexImage(size_t width, size_t height) 
    : width_(width), height_(height), data_(width * height) {}

ComplexImage::ComplexImage(const std::vector<uint8_t>& grayscale_data, size_t width, size_t height)
    : width_(width), height_(height), data_(width * height) {
    setFromGrayscale(grayscale_data, width, height);
}

void ComplexImage::resize(size_t width, size_t height) {
    width_ = width;
    height_ = height;
    data_.resize(width * height);
}

void ComplexImage::setFromGrayscale(const std::vector<uint8_t>& grayscale_data, size_t width, size_t height) {
    resize(width, height);
    for (size_t i = 0; i < data_.size(); ++i) {
        data_[i] = Complex(grayscale_data[i] / 255.0, 0.0);
    }
}

ComplexImage::Complex& ComplexImage::at(size_t x, size_t y) {
    return data_[index(x, y)];
}

const ComplexImage::Complex& ComplexImage::at(size_t x, size_t y) const {
    return data_[index(x, y)];
}

std::vector<double> ComplexImage::getMagnitudeImage() const {
    std::vector<double> magnitude(data_.size());
    std::transform(data_.begin(), data_.end(), magnitude.begin(),
                   [](const Complex& c) { return std::abs(c); });
    return magnitude;
}

std::vector<double> ComplexImage::getPhaseImage() const {
    std::vector<double> phase(data_.size());
    std::transform(data_.begin(), data_.end(), phase.begin(),
                   [](const Complex& c) { return std::arg(c); });
    return phase;
}

std::vector<uint8_t> ComplexImage::getGrayscaleFromReal() const {
    std::vector<uint8_t> grayscale(data_.size());
    
    double min_val = std::numeric_limits<double>::max();
    double max_val = std::numeric_limits<double>::lowest();
    
    for (const auto& c : data_) {
        double real = c.real();
        min_val = std::min(min_val, real);
        max_val = std::max(max_val, real);
    }
    
    double range = max_val - min_val;
    if (range < 1e-10) range = 1.0;
    
    for (size_t i = 0; i < data_.size(); ++i) {
        double normalized = (data_[i].real() - min_val) / range;
        grayscale[i] = static_cast<uint8_t>(std::clamp(normalized * 255.0, 0.0, 255.0));
    }
    
    return grayscale;
}

void ComplexImage::normalize() {
    double factor = 1.0 / std::sqrt(width_ * height_);
    for (auto& c : data_) {
        c *= factor;
    }
}

void ComplexImage::fftShift() {
    size_t half_width = width_ / 2;
    size_t half_height = height_ / 2;
    
    for (size_t y = 0; y < half_height; ++y) {
        for (size_t x = 0; x < half_width; ++x) {
            std::swap(at(x, y), at(x + half_width, y + half_height));
            std::swap(at(x + half_width, y), at(x, y + half_height));
        }
    }
}

void ComplexImage::ifftShift() {
    fftShift();
}

size_t ComplexImage::index(size_t x, size_t y) const {
    return y * width_ + x;
}