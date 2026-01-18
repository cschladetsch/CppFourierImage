#include <algorithm>
#include <iostream>
#include <cmath>
#include <ranges>
#include <execution>

#include "FourierVisualizer.hpp"
#include "FourierTransform.hpp"

FourierVisualizer::FourierVisualizer() : fourier_transform_(std::make_shared<FourierTransform>()) {
    frequencyChangeHandlerId_ = EventDispatcher::getInstance().subscribe<FrequencyChangeEvent>(
        [this](const FrequencyChangeEvent& event) {
            if (animation_state_.current_frequency_count != event.newFrequencyCount) {
                setFrequencyCount(event.newFrequencyCount);
            }
        }
    );
}

FourierVisualizer::~FourierVisualizer() {
    EventDispatcher::getInstance().unsubscribe<FrequencyChangeEvent>(frequencyChangeHandlerId_);
}

void FourierVisualizer::setImage(const ComplexImage& frequency_domain) {
    frequency_domain_ = frequency_domain;
    resetAnimationState(false);
    animation_state_.reconstructed_image = ComplexImage(frequency_domain.getWidth(), frequency_domain.getHeight());
}

void FourierVisualizer::setRGBImage(const RGBComplexImage& frequency_domain) {
    rgb_frequency_domain_ = frequency_domain;
    resetAnimationState(true);
    animation_state_.reconstructed_rgb_image = RGBComplexImage(frequency_domain.getWidth(), frequency_domain.getHeight());
}

void FourierVisualizer::resetAnimationState(bool is_rgb) {
    animation_state_.active_frequencies.clear();
    animation_state_.current_frequency_count = 0;
    animation_state_.time_accumulator = 0.0f;
    animation_state_.is_rgb = is_rgb;
}

void FourierVisualizer::setFrequencyCount(size_t count) {
    if (count != animation_state_.current_frequency_count) {
        animation_state_.current_frequency_count = count;
        if (animation_state_.is_rgb) {
            reconstructRGBFromFrequencies();
        } else {
            reconstructFromFrequencies();
        }
    }
}

void FourierVisualizer::updateAnimation(Scalar delta_time) {
    if (!animation_state_.is_animating) return;
    
    animation_state_.time_accumulator += delta_time * animation_state_.animation_speed;
    
    size_t target_count = static_cast<size_t>(animation_state_.time_accumulator * 10);
    target_count = std::min(target_count, static_cast<size_t>(frequency_domain_.getWidth() * frequency_domain_.getHeight()));
    
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

std::vector<Scalar> FourierVisualizer::getMagnitudeSpectrum() const {
    return frequency_domain_.getMagnitudeImage();
}

std::vector<Scalar> FourierVisualizer::getPhaseSpectrum() const {
    return frequency_domain_.getPhaseImage();
}

std::vector<std::pair<Scalar, Scalar>> FourierVisualizer::getFrequencyPath() const {
    std::vector<std::pair<Scalar, Scalar>> path;
    path.reserve(animation_state_.active_frequencies.size());
    
    for (const auto& [x, y] : animation_state_.active_frequencies) {
        path.emplace_back(static_cast<Scalar>(x), static_cast<Scalar>(y));
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
    
    RGBComplexImage filtered = fourier_transform_->keepTopFrequenciesRGB(rgb_frequency_domain_, 
                                                                         animation_state_.current_frequency_count);
    
    animation_state_.reconstructed_rgb_image = fourier_transform_->transformRGB2D(filtered, 
                                                                                 FourierTransform::Direction::Inverse);
    
    animation_state_.active_frequencies = fourier_transform_->getTopFrequencyIndices(rgb_frequency_domain_.getChannel(0), 
                                                                                    animation_state_.current_frequency_count);
}

std::vector<FourierVisualizer::VisualizationLine> FourierVisualizer::getVisualizationLines(Scalar width, Scalar height) const {
    if (animation_state_.is_rgb) {
        const auto& freqDomain = rgb_frequency_domain_.getChannel(0);
        if (freqDomain.getWidth() <= 0) {
            return {};
        }
        return buildVisualizationLines(freqDomain, width, height);
    }

    if (frequency_domain_.getWidth() <= 0) {
        return {};
    }

    return buildVisualizationLines(frequency_domain_, width, height);
}

std::vector<FourierVisualizer::VisualizationLine> FourierVisualizer::buildVisualizationLines(const ComplexImage& freqDomain, Scalar width, Scalar height) const {
    std::vector<VisualizationLine> lines;
    lines.reserve(animation_state_.active_frequencies.size());

    const Scalar freq_width = freqDomain.getWidth();
    const Scalar freq_height = freqDomain.getHeight();

    const Scalar centerX = freq_width / 2.0;
    const Scalar centerY = freq_height / 2.0;
    
    const Scalar scaleX = width / freq_width;
    const Scalar scaleY = height / freq_height;
    
    auto visualizationLines = animation_state_.active_frequencies 
        | std::views::transform([&](const auto& freq_pair) {
            const auto [u, v] = freq_pair;

            Scalar screenX = (static_cast<Scalar>(u) + centerX) * scaleX;
            Scalar screenY = (static_cast<Scalar>(v) + centerY) * scaleY;

            screenX = std::clamp(screenX, Scalar(0.0), width);
            screenY = std::clamp(screenY, Scalar(0.0), height);

            const auto complex_val = freqDomain.at(u, v);
            const float magnitude = std::abs(complex_val);
            const float phase = std::arg(complex_val);

            return VisualizationLine{
                .x1 = width * 0.5,
                .y1 = height * 0.5,
                .x2 = screenX,
                .y2 = screenY,
                .magnitude = magnitude,
                .phase = phase,
                .frequency = std::sqrt(static_cast<Scalar>(u * u + v * v))
            };
        });

    std::ranges::copy(visualizationLines, std::back_inserter(lines));
    return lines;
}
