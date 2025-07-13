#pragma once

#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"
#include "Types.hpp"
#include <vector>
#include <memory>

class FourierTransform;

class FourierVisualizer {
public:
    struct AnimationState {
        std::vector<std::pair<int, int>> active_frequencies;
        ComplexImage reconstructed_image;
        RGBComplexImage reconstructed_rgb_image;
        size_t current_frequency_count = 0;
        bool is_animating = false;
        Scalar animation_speed = 1.0;
        Scalar time_accumulator = 0.0;
        bool is_rgb = false;
    };
    
    struct VisualizationLine {
        Scalar x1, y1, x2, y2;  // Line endpoints in screen coordinates
        Scalar magnitude;       // Magnitude of the frequency component
        Scalar phase;          // Phase of the frequency component
        Scalar frequency;      // Frequency value
    };
    
    FourierVisualizer();
    ~FourierVisualizer();
    
    void setImage(const ComplexImage& frequency_domain);
    void setRGBImage(const RGBComplexImage& frequency_domain);
    void setFrequencyCount(size_t count);
    void updateAnimation(Scalar delta_time);
    
    ComplexImage getReconstructedImage() const;
    RGBComplexImage getReconstructedRGBImage() const;
    const AnimationState& getAnimationState() const { return animation_state_; }
    AnimationState& getAnimationState() { return animation_state_; }
    
    std::vector<Scalar> getMagnitudeSpectrum() const;
    std::vector<Scalar> getPhaseSpectrum() const;
    std::vector<std::pair<Scalar, Scalar>> getFrequencyPath() const;
    std::vector<VisualizationLine> getVisualizationLines(Scalar width, Scalar height) const;
    
private:
    ComplexImage frequency_domain_;
    RGBComplexImage rgb_frequency_domain_;
    AnimationState animation_state_;
    std::shared_ptr<FourierTransform> fourier_transform_;
    
    void reconstructFromFrequencies();
    void reconstructRGBFromFrequencies();
};