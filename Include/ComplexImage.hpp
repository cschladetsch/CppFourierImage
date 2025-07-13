#pragma once

#include <vector>
#include <cstdint>
#include "Types.hpp"

class ComplexImage {
public:
    using Complex = ::Complex;
    
    ComplexImage() = default;
    ComplexImage(Scalar width, Scalar height);
    ComplexImage(const std::vector<uint8_t>& grayscale_data, Scalar width, Scalar height);
    
    void resize(Scalar width, Scalar height);
    void setFromGrayscale(const std::vector<uint8_t>& grayscale_data, Scalar width, Scalar height);
    
    Complex& at(size_t x, size_t y);
    const Complex& at(size_t x, size_t y) const;
    
    Scalar getWidth() const { return width_; }
    Scalar getHeight() const { return height_; }
    
    std::vector<Scalar> getMagnitudeImage() const;
    std::vector<Scalar> getPhaseImage() const;
    std::vector<uint8_t> getGrayscaleFromReal() const;
    
    void normalize();
    void fftShift();
    void ifftShift();
    
    const std::vector<Complex>& getData() const { return data_; }
    std::vector<Complex>& getData() { return data_; }
    
private:
    Scalar width_ = 0.0;
    Scalar height_ = 0.0;
    std::vector<Complex> data_;
    
    size_t index(size_t x, size_t y) const;
};