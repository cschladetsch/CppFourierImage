#pragma once

#include "ComplexImage.hpp"
#include <vector>
#include <cstdint>

class ImageProcessor {
public:
    static ComplexImage padToPowerOfTwo(const ComplexImage& input);
    static ComplexImage cropToOriginalSize(const ComplexImage& input, size_t original_width, size_t original_height);
    
    static std::vector<uint8_t> normalizeToUint8(const std::vector<double>& data);
    static std::vector<float> normalizeToFloat(const std::vector<double>& data);
    
    static ComplexImage applyGaussianBlur(const ComplexImage& input, double sigma);
    static ComplexImage applyEdgeDetection(const ComplexImage& input);
    
    static void applyLogScale(std::vector<double>& magnitude_data);
    static void applyColorMap(const std::vector<uint8_t>& grayscale, std::vector<uint8_t>& rgb_output);
};