#pragma once

#include <complex>
#include <vector>
#include <cstdint>

class ComplexImage {
public:
    using Complex = std::complex<double>;
    
    ComplexImage() = default;
    ComplexImage(size_t width, size_t height);
    ComplexImage(const std::vector<uint8_t>& grayscale_data, size_t width, size_t height);
    
    void resize(size_t width, size_t height);
    void setFromGrayscale(const std::vector<uint8_t>& grayscale_data, size_t width, size_t height);
    
    Complex& at(size_t x, size_t y);
    const Complex& at(size_t x, size_t y) const;
    
    size_t getWidth() const { return width_; }
    size_t getHeight() const { return height_; }
    
    std::vector<double> getMagnitudeImage() const;
    std::vector<double> getPhaseImage() const;
    std::vector<uint8_t> getGrayscaleFromReal() const;
    
    void normalize();
    void fftShift();
    void ifftShift();
    
    const std::vector<Complex>& getData() const { return data_; }
    std::vector<Complex>& getData() { return data_; }
    
private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<Complex> data_;
    
    size_t index(size_t x, size_t y) const;
};