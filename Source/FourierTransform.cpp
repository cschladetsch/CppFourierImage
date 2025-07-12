#include "FourierTransform.hpp"
#include "ImageProcessor.hpp"
#include "RgbComplexImage.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <execution>

FourierTransform::FourierTransform() = default;
FourierTransform::~FourierTransform() = default;

ComplexImage FourierTransform::transform2D(const ComplexImage& input, Direction direction) {
    // Pad to power of 2 for efficient FFT
    ComplexImage padded = ImageProcessor::padToPowerOfTwo(input);
    fft2D(padded, direction);
    
    // For inverse transform, crop back to original size
    if (direction == Direction::Inverse) {
        return ImageProcessor::cropToOriginalSize(padded, input.getWidth(), input.getHeight());
    }
    
    return padded;
}

void FourierTransform::fft2D(ComplexImage& image, Direction direction) {
    size_t width = image.getWidth();
    size_t height = image.getHeight();
    
    std::vector<ComplexImage::Complex> row(width);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            row[x] = image.at(x, y);
        }
        fft1D(row, direction);
        for (size_t x = 0; x < width; ++x) {
            image.at(x, y) = row[x];
        }
    }
    
    std::vector<ComplexImage::Complex> col(height);
    for (size_t x = 0; x < width; ++x) {
        for (size_t y = 0; y < height; ++y) {
            col[y] = image.at(x, y);
        }
        fft1D(col, direction);
        for (size_t y = 0; y < height; ++y) {
            image.at(x, y) = col[y];
        }
    }
}

void FourierTransform::fft1D(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    if (n <= 1) return;
    
    if ((n & (n - 1)) == 0) {
        cooleyTukeyFFT(data, direction);
    } else {
        dft(data, direction);
    }
}

void FourierTransform::cooleyTukeyFFT(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    if (n <= 1) return;
    
    for (size_t i = 0, j = 0; i < n; ++i) {
        if (i < j) std::swap(data[i], data[j]);
        size_t bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
    }
    
    double angle_sign = (direction == Direction::Forward) ? -2.0 * PI : 2.0 * PI;
    
    for (size_t len = 2; len <= n; len <<= 1) {
        double angle = angle_sign / len;
        ComplexImage::Complex wlen(std::cos(angle), std::sin(angle));
        
        for (size_t i = 0; i < n; i += len) {
            ComplexImage::Complex w(1, 0);
            size_t half_len = len >> 1;
            
            for (size_t j = 0; j < half_len; ++j) {
                ComplexImage::Complex u = data[i + j];
                ComplexImage::Complex v = data[i + j + half_len] * w;
                data[i + j] = u + v;
                data[i + j + half_len] = u - v;
                w *= wlen;
            }
        }
    }
    
    if (direction == Direction::Inverse) {
        double factor = 1.0 / n;
        for (auto& c : data) {
            c *= factor;
        }
    }
}

void FourierTransform::dft(std::span<ComplexImage::Complex> data, Direction direction) {
    size_t n = data.size();
    std::vector<ComplexImage::Complex> result(n);
    
    double angle_sign = (direction == Direction::Forward) ? -2.0 * PI : 2.0 * PI;
    
    for (size_t k = 0; k < n; ++k) {
        ComplexImage::Complex sum(0, 0);
        for (size_t j = 0; j < n; ++j) {
            double angle = angle_sign * k * j / n;
            sum += data[j] * ComplexImage::Complex(std::cos(angle), std::sin(angle));
        }
        result[k] = sum;
    }
    
    if (direction == Direction::Inverse) {
        double factor = 1.0 / n;
        for (auto& c : result) {
            c *= factor;
        }
    }
    
    std::copy(result.begin(), result.end(), data.begin());
}

ComplexImage FourierTransform::applyFrequencyMask(const ComplexImage& frequency_domain, double frequency_cutoff, bool low_pass) {
    ComplexImage result = frequency_domain;
    size_t width = result.getWidth();
    size_t height = result.getHeight();
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double fx = (x < width / 2) ? x : x - width;
            double fy = (y < height / 2) ? y : y - height;
            double freq = std::sqrt(fx * fx + fy * fy);
            
            if ((low_pass && freq > frequency_cutoff) || (!low_pass && freq < frequency_cutoff)) {
                result.at(x, y) = ComplexImage::Complex(0, 0);
            }
        }
    }
    
    return result;
}

ComplexImage FourierTransform::applyFrequencyMaskCircular(const ComplexImage& frequency_domain, double radius_ratio) {
    ComplexImage result = frequency_domain;
    size_t width = result.getWidth();
    size_t height = result.getHeight();
    
    double cx = width / 2.0;
    double cy = height / 2.0;
    double max_radius = std::min(cx, cy) * radius_ratio;
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist > max_radius) {
                result.at(x, y) = ComplexImage::Complex(0, 0);
            }
        }
    }
    
    return result;
}

std::vector<std::pair<int, int>> FourierTransform::getTopFrequencyIndices(const ComplexImage& frequency_domain, int num_frequencies) {
    size_t width = frequency_domain.getWidth();
    size_t height = frequency_domain.getHeight();
    
    std::vector<std::tuple<double, int, int>> magnitude_indices;
    magnitude_indices.reserve(width * height);
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            double mag = std::abs(frequency_domain.at(x, y));
            magnitude_indices.emplace_back(mag, x, y);
        }
    }
    
    std::partial_sort(magnitude_indices.begin(), 
                      magnitude_indices.begin() + std::min(static_cast<size_t>(num_frequencies), magnitude_indices.size()),
                      magnitude_indices.end(),
                      std::greater<>{});
    
    std::vector<std::pair<int, int>> result;
    result.reserve(num_frequencies);
    
    size_t limit = std::min(static_cast<size_t>(num_frequencies), magnitude_indices.size());
    for (size_t i = 0; i < limit; ++i) {
        result.emplace_back(std::get<1>(magnitude_indices[i]), std::get<2>(magnitude_indices[i]));
    }
    
    return result;
}

ComplexImage FourierTransform::keepTopFrequencies(const ComplexImage& frequency_domain, int num_frequencies) {
    ComplexImage result(frequency_domain.getWidth(), frequency_domain.getHeight());
    auto top_indices = getTopFrequencyIndices(frequency_domain, num_frequencies);
    
    for (const auto& [x, y] : top_indices) {
        result.at(x, y) = frequency_domain.at(x, y);
    }
    
    return result;
}

RGBComplexImage FourierTransform::transformRGB2D(const RGBComplexImage& input, Direction direction) {
    RGBComplexImage result(input.getWidth(), input.getHeight());
    
    // Transform each channel separately
    for (int channel = 0; channel < 3; ++channel) {
        ComplexImage transformed = transform2D(input.getChannel(channel), direction);
        result.getChannel(channel) = transformed;
    }
    
    return result;
}

RGBComplexImage FourierTransform::keepTopFrequenciesRGB(const RGBComplexImage& frequency_domain, int num_frequencies) {
    RGBComplexImage result(frequency_domain.getWidth(), frequency_domain.getHeight());
    
    // Apply frequency filtering to each channel
    for (int channel = 0; channel < 3; ++channel) {
        result.getChannel(channel) = keepTopFrequencies(frequency_domain.getChannel(channel), num_frequencies);
    }
    
    return result;
}