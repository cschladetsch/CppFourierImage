#include "fourier_visualizer.h"
#include "fourier_transform.h"
#include <algorithm>

FourierVisualizer::FourierVisualizer() = default;
FourierVisualizer::~FourierVisualizer() = default;

void FourierVisualizer::setImage(const ComplexImage& frequency_domain) {
    frequency_domain_ = frequency_domain;
    animation_state_.active_frequencies.clear();
    animation_state_.current_frequency_count = 0;
    animation_state_.time_accumulator = 0.0f;
    
    // Initialize with an empty reconstruction
    animation_state_.reconstructed_image = ComplexImage(frequency_domain.getWidth(), frequency_domain.getHeight());
}

void FourierVisualizer::setFrequencyCount(int count) {
    if (count != animation_state_.current_frequency_count) {
        animation_state_.current_frequency_count = count;
        reconstructFromFrequencies();
    }
}

void FourierVisualizer::updateAnimation(float delta_time) {
    if (!animation_state_.is_animating) return;
    
    animation_state_.time_accumulator += delta_time * animation_state_.animation_speed;
    
    int target_count = static_cast<int>(animation_state_.time_accumulator * 10);
    target_count = std::min(target_count, static_cast<int>(frequency_domain_.getWidth() * frequency_domain_.getHeight()));
    
    if (target_count != animation_state_.current_frequency_count) {
        setFrequencyCount(target_count);
    }
}

ComplexImage FourierVisualizer::getReconstructedImage() const {
    return animation_state_.reconstructed_image;
}

void FourierVisualizer::reconstructFromFrequencies() {
    FourierTransform transform;
    
    ComplexImage filtered_freq = transform.keepTopFrequencies(frequency_domain_, animation_state_.current_frequency_count);
    
    animation_state_.reconstructed_image = transform.transform2D(filtered_freq, FourierTransform::Direction::Inverse);
    
    animation_state_.active_frequencies = transform.getTopFrequencyIndices(frequency_domain_, animation_state_.current_frequency_count);
}

std::vector<float> FourierVisualizer::getMagnitudeSpectrum() const {
    auto magnitude = frequency_domain_.getMagnitudeImage();
    std::vector<float> spectrum(magnitude.size());
    std::transform(magnitude.begin(), magnitude.end(), spectrum.begin(),
                   [](double v) { return static_cast<float>(v); });
    return spectrum;
}

std::vector<float> FourierVisualizer::getPhaseSpectrum() const {
    auto phase = frequency_domain_.getPhaseImage();
    std::vector<float> spectrum(phase.size());
    std::transform(phase.begin(), phase.end(), spectrum.begin(),
                   [](double v) { return static_cast<float>(v); });
    return spectrum;
}

std::vector<std::pair<float, float>> FourierVisualizer::getFrequencyPath() const {
    std::vector<std::pair<float, float>> path;
    path.reserve(animation_state_.active_frequencies.size());
    
    for (const auto& [x, y] : animation_state_.active_frequencies) {
        path.emplace_back(static_cast<float>(x), static_cast<float>(y));
    }
    
    return path;
}