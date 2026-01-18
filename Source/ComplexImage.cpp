#include "ComplexImage.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

ComplexImage::ComplexImage(Scalar width, Scalar height) 
    : width_(width), height_(height), data_(static_cast<size_t>(width * height)) {}

ComplexImage::ComplexImage(const std::vector<uint8_t>& grayscale_data, Scalar width, Scalar height)
    : width_(width), height_(height), data_(static_cast<size_t>(width * height)) {
    setFromGrayscale(grayscale_data, width, height);
}

void ComplexImage::resize(Scalar width, Scalar height) {
    width_ = width;
    height_ = height;
    data_.resize(static_cast<size_t>(width * height));
}

void ComplexImage::setFromGrayscale(const std::vector<uint8_t>& grayscale_data, Scalar width, Scalar height) {
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

std::vector<Scalar> ComplexImage::getMagnitudeImage() const {
    std::vector<Scalar> magnitude(data_.size());
    std::transform(data_.begin(), data_.end(), magnitude.begin(),
                   [](const Complex& c) { return std::abs(c); });
    return magnitude;
}

std::vector<Scalar> ComplexImage::getPhaseImage() const {
    std::vector<Scalar> phase(data_.size());
    std::transform(data_.begin(), data_.end(), phase.begin(),
                   [](const Complex& c) { return std::arg(c); });
    return phase;
}

std::vector<uint8_t> ComplexImage::getGrayscaleFromReal() const {
    std::vector<uint8_t> grayscale(data_.size());
    
    Scalar min_val = std::numeric_limits<Scalar>::max();
    Scalar max_val = std::numeric_limits<Scalar>::lowest();
    
    for (const auto& c : data_) {
        Scalar real = c.real();
        min_val = std::min(min_val, real);
        max_val = std::max(max_val, real);
    }
    
    Scalar range = max_val - min_val;
    if (range < 1e-10) range = 1.0;
    
    for (size_t i = 0; i < data_.size(); ++i) {
        Scalar normalized = (data_[i].real() - min_val) / range;
        grayscale[i] = static_cast<uint8_t>(std::clamp(normalized * 255.0, 0.0, 255.0));
    }
    
    return grayscale;
}

void ComplexImage::normalize() {
    Scalar max_magnitude = 0.0;
    for (const auto& c : data_) {
        max_magnitude = std::max(max_magnitude, std::abs(c));
    }

    if (max_magnitude <= std::numeric_limits<Scalar>::epsilon()) {
        return;
    }

    for (auto& c : data_) {
        c /= max_magnitude;
    }
}

void ComplexImage::fftShift() {
    size_t half_width = static_cast<size_t>(width_) / 2;
    size_t half_height = static_cast<size_t>(height_) / 2;
    
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
    return y * static_cast<size_t>(width_) + x;
}
