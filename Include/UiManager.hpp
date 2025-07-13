#pragma once

#include <memory>
#include <string>
#include <vector>
#include <array>
#include "EventSystem.hpp"

class ComplexImage;
class RGBComplexImage;

class ImageLoader;
class FourierTransform;
class FourierVisualizer;
class Renderer;

class UIManager {
public:
    UIManager(std::shared_ptr<ImageLoader> imageLoader,
              std::shared_ptr<FourierTransform> fourierTransform,
              std::shared_ptr<FourierVisualizer> visualizer,
              std::shared_ptr<Renderer> renderer,
              size_t maxImageSize = 512);
    ~UIManager();

    void initialize();
    void update();
    void render();
    void handleInput();

private:
    void loadImage(const std::string& filepath);
    void updateVisualization();
    void scanResourcesFolder();
    void renderSpectrumWindow();
    void computeChannelSpectrums();

    // Component references
    std::shared_ptr<ImageLoader> imageLoader_;
    std::shared_ptr<FourierTransform> fourierTransform_;
    std::shared_ptr<FourierVisualizer> visualizer_;
    std::shared_ptr<Renderer> renderer_;

    // UI state
    std::vector<std::string> availableImages_;
    size_t selectedImageIndex_ = 0;
    bool imageLoaded_ = false;
    size_t imageWidth_ = 0;
    size_t imageHeight_ = 0;

    // Fourier parameters
    size_t frequencyCount_;
    size_t maxFrequencies_;
    std::shared_ptr<ComplexImage> transformedImage_;
    std::shared_ptr<RGBComplexImage> transformedRGBImage_;
    bool useRGB_ = true;  // Default to RGB processing


    
    // Processing parameters
    size_t maxImageSize_;
    
    // Spectrum visualization data
    std::array<std::vector<float>, 3> channelSpectrums_; // RGB channels
    bool showSpectrumWindow_ = true;
    
    // Event handling
    EventDispatcher::HandlerId frequencyChangeHandlerId_;
};