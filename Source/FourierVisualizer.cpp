#include "FourierVisualizer.hpp"
#include "FourierTransform.hpp"
#include <algorithm>

FourierVisualizer::FourierVisualizer() : fourier_transform_(std::make_shared<FourierTransform>()) {}
FourierVisualizer::~FourierVisualizer() = default;

void FourierVisualizer::setImage(const ComplexImage& frequency_domain) {
    frequency_domain_ = frequency_domain;
    animation_state_.active_frequencies.clear();
    animation_state_.current_frequency_count = 0;
    animation_state_.time_accumulator = 0.0f;
    animation_state_.is_rgb = false;
    
    // Initialize with an empty reconstruction
    animation_state_.reconstructed_image = ComplexImage(frequency_domain.getWidth(), frequency_domain.getHeight());
}

void FourierVisualizer::setRGBImage(const RGBComplexImage& frequency_domain) {
    rgb_frequency_domain_ = frequency_domain;
    animation_state_.active_frequencies.clear();
    animation_state_.current_frequency_count = 0;
    animation_state_.time_accumulator = 0.0f;
    animation_state_.is_rgb = true;
    
    // Initialize with an empty RGB reconstruction
    animation_state_.reconstructed_rgb_image = RGBComplexImage(frequency_domain.getWidth(), frequency_domain.getHeight());
}

void FourierVisualizer::setFrequencyCount(int count) {
    if (count != animation_state_.current_frequency_count) {
        animation_state_.current_frequency_count = count;
        if (animation_state_.is_rgb) {
            reconstructRGBFromFrequencies();
        } else {
            reconstructFromFrequencies();
        }
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

ComplexImage FourierVisualizer::getReconstructedImage() const {
    return animation_state_.reconstructed_image;
}

RGBComplexImage FourierVisualizer::getReconstructedRGBImage() const {
    return animation_state_.reconstructed_rgb_image;
}

void FourierVisualizer::reconstructRGBFromFrequencies() {
    if (!fourier_transform_) return;
    
    // Keep top frequencies for each channel
    RGBComplexImage filtered = fourier_transform_->keepTopFrequenciesRGB(rgb_frequency_domain_, 
                                                                         animation_state_.current_frequency_count);
    
    // Inverse transform to get spatial domain
    animation_state_.reconstructed_rgb_image = fourier_transform_->transformRGB2D(filtered, 
                                                                                 FourierTransform::Direction::Inverse);
}