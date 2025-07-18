#pragma once

#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"
#include "Types.hpp"
#include <memory>
#include <span>

class FourierTransform {
public:
    enum class Direction {
        Forward,
        Inverse
    };
    
    FourierTransform();
    ~FourierTransform();
    
    ComplexImage transform2D(const ComplexImage& input, Direction direction = Direction::Forward);
    
    // RGB version - transforms each channel separately
    RGBComplexImage transformRGB2D(const RGBComplexImage& input, Direction direction = Direction::Forward);
    
    ComplexImage applyFrequencyMask(const ComplexImage& frequency_domain, Scalar frequency_cutoff, bool low_pass = true);
    ComplexImage applyFrequencyMaskCircular(const ComplexImage& frequency_domain, Scalar radius_ratio);
    ComplexImage keepTopFrequencies(const ComplexImage& frequency_domain, size_t num_frequencies);
    
    // RGB versions
    RGBComplexImage keepTopFrequenciesRGB(const RGBComplexImage& frequency_domain, size_t num_frequencies);
    
    std::vector<std::pair<int, int>> getTopFrequencyIndices(const ComplexImage& frequency_domain, size_t num_frequencies);
    
private:
    void fft1D(std::span<ComplexImage::Complex> data, Direction direction);
    void fft2D(ComplexImage& image, Direction direction);
    void cooleyTukeyFFT(std::span<ComplexImage::Complex> data, Direction direction);
    void dft(std::span<ComplexImage::Complex> data, Direction direction);
    
    static constexpr Scalar PI = 3.14159265358979323846;
};