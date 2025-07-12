#pragma once

#include "complex_image.h"
#include <vector>
#include <memory>

class FourierVisualizer {
public:
    struct AnimationState {
        std::vector<std::pair<int, int>> active_frequencies;
        ComplexImage reconstructed_image;
        int current_frequency_count = 0;
        bool is_animating = false;
        float animation_speed = 1.0f;
        float time_accumulator = 0.0f;
    };
    
    FourierVisualizer();
    ~FourierVisualizer();
    
    void setImage(const ComplexImage& frequency_domain);
    void setFrequencyCount(int count);
    void updateAnimation(float delta_time);
    
    ComplexImage getReconstructedImage() const;
    const AnimationState& getAnimationState() const { return animation_state_; }
    AnimationState& getAnimationState() { return animation_state_; }
    
    std::vector<float> getMagnitudeSpectrum() const;
    std::vector<float> getPhaseSpectrum() const;
    std::vector<std::pair<float, float>> getFrequencyPath() const;
    
private:
    ComplexImage frequency_domain_;
    AnimationState animation_state_;
    
    void reconstructFromFrequencies();
};