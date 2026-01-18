#include "ImageProcessor.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>

ComplexImage ImageProcessor::padToPowerOfTwo(const ComplexImage& input) {
    size_t width = input.getWidth();
    size_t height = input.getHeight();
    
    size_t padded_width = 1;
    while (padded_width < width) padded_width <<= 1;
    
    size_t padded_height = 1;
    while (padded_height < height) padded_height <<= 1;
    
    if (padded_width == width && padded_height == height) {
        return input;
    }
    
    ComplexImage padded(padded_width, padded_height);
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            padded.at(x, y) = input.at(x, y);
        }
    }
    
    return padded;
}

ComplexImage ImageProcessor::cropToOriginalSize(const ComplexImage& input, size_t original_width, size_t original_height) {
    ComplexImage cropped(original_width, original_height);
    
    for (size_t y = 0; y < original_height; ++y) {
        for (size_t x = 0; x < original_width; ++x) {
            cropped.at(x, y) = input.at(x, y);
        }
    }
    
    return cropped;
}

std::vector<uint8_t> ImageProcessor::normalizeToUint8(const std::vector<Scalar>& data) {
    std::vector<uint8_t> result(data.size());
    
    auto [min_it, max_it] = std::minmax_element(data.begin(), data.end());
    Scalar min_val = *min_it;
    Scalar max_val = *max_it;
    Scalar range = max_val - min_val;
    
    if (range < 1e-10) range = 1.0;
    
    std::transform(data.begin(), data.end(), result.begin(),
                   [min_val, range](Scalar val) {
                       Scalar normalized = (val - min_val) / range;
                       return static_cast<uint8_t>(std::clamp(normalized * 255.0, 0.0, 255.0));
                   });
    
    return result;
}

std::vector<Scalar> ImageProcessor::normalizeToFloat(const std::vector<Scalar>& data) {
    std::vector<Scalar> result(data.size());
    
    auto [min_it, max_it] = std::minmax_element(data.begin(), data.end());
    Scalar min_val = *min_it;
    Scalar max_val = *max_it;
    Scalar range = max_val - min_val;
    
    if (range < 1e-10) range = 1.0;
    
    std::transform(data.begin(), data.end(), result.begin(),
                   [min_val, range](Scalar val) {
                       return (val - min_val) / range;
                   });
    
    return result;
}

void ImageProcessor::applyLogScale(std::vector<Scalar>& magnitude_data) {
    std::transform(magnitude_data.begin(), magnitude_data.end(), magnitude_data.begin(),
                   [](Scalar val) { return std::log10(1.0 + val); });
}

void ImageProcessor::applyColorMap(const std::vector<uint8_t>& grayscale, std::vector<uint8_t>& rgb_output, ColorMap map) {
    rgb_output.resize(grayscale.size() * 3);

    if (map == ColorMap::Jet) {
        for (size_t i = 0; i < grayscale.size(); ++i) {
            const uint8_t val = grayscale[i];
            const Scalar t = val / 255.0;

            uint8_t r = 0, g = 0, b = 0;
            if (t < 0.25f) {
                g = static_cast<uint8_t>(t * 4 * 255);
                b = 255;
            } else if (t < 0.5f) {
                g = 255;
                b = static_cast<uint8_t>((1.0f - (t - 0.25f) * 4) * 255);
            } else if (t < 0.75f) {
                r = static_cast<uint8_t>((t - 0.5f) * 4 * 255);
                g = 255;
            } else {
                r = 255;
                g = static_cast<uint8_t>((1.0f - (t - 0.75f) * 4) * 255);
            }

            rgb_output[i * 3] = r;
            rgb_output[i * 3 + 1] = g;
            rgb_output[i * 3 + 2] = b;
        }
        return;
    }

    for (size_t i = 0; i < grayscale.size(); ++i) {
        uint8_t val = grayscale[i];
        rgb_output[i * 3] = val;
        rgb_output[i * 3 + 1] = val;
        rgb_output[i * 3 + 2] = val;
    }
}

ComplexImage ImageProcessor::applyGaussianBlur(const ComplexImage& input, Scalar sigma) {
    if (sigma <= 0.0) {
        return input;
    }

    size_t width = input.getWidth();
    size_t height = input.getHeight();
    ComplexImage result(width, height);
    
    int kernel_size = static_cast<int>(6 * sigma + 1);
    if (kernel_size % 2 == 0) kernel_size++;
    int half_size = kernel_size / 2;
    
    std::vector<Scalar> kernel(kernel_size);
    Scalar sum = 0.0;
    for (int i = 0; i < kernel_size; ++i) {
        int x = i - half_size;
        kernel[i] = std::exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }
    for (auto& k : kernel) k /= sum;
    
    ComplexImage temp(width, height);
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            ComplexImage::Complex sum(0, 0);
            for (int k = 0; k < kernel_size; ++k) {
                ptrdiff_t sx = static_cast<ptrdiff_t>(x) + k - half_size;
                if (sx >= 0 && sx < static_cast<ptrdiff_t>(width)) {
                    sum += input.at(sx, y) * kernel[k];
                }
            }
            temp.at(x, y) = sum;
        }
    }
    
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            ComplexImage::Complex sum(0, 0);
            for (int k = 0; k < kernel_size; ++k) {
                ptrdiff_t sy = static_cast<ptrdiff_t>(y) + k - half_size;
                if (sy >= 0 && sy < static_cast<ptrdiff_t>(height)) {
                    sum += temp.at(x, sy) * kernel[k];
                }
            }
            result.at(x, y) = sum;
        }
    }
    
    return result;
}

ComplexImage ImageProcessor::applyEdgeDetection(const ComplexImage& input) {
    size_t width = input.getWidth();
    size_t height = input.getHeight();
    ComplexImage result(width, height);
    
    const int sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    const int sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    
    for (size_t y = 1; y < height - 1; ++y) {
        for (size_t x = 1; x < width - 1; ++x) {
            ComplexImage::Complex gx(0, 0), gy(0, 0);
            
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    ComplexImage::Complex pixel = input.at(x + dx, y + dy);
                    gx += pixel * static_cast<Scalar>(sobel_x[dy + 1][dx + 1]);
                    gy += pixel * static_cast<Scalar>(sobel_y[dy + 1][dx + 1]);
                }
            }
            
            Scalar magnitude = std::sqrt(std::norm(gx) + std::norm(gy));
            result.at(x, y) = ComplexImage::Complex(magnitude, 0);
        }
    }
    
    return result;
}
