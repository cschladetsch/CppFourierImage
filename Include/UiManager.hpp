#pragma once

#include <memory>
#include <string>
#include <vector>

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
              int maxImageSize = 512);
    ~UIManager();

    void initialize();
    void update();
    void render();
    void handleInput();

private:
    void loadImage(const std::string& filepath);
    void updateVisualization();
    void scanResourcesFolder();

    // Component references
    std::shared_ptr<ImageLoader> imageLoader_;
    std::shared_ptr<FourierTransform> fourierTransform_;
    std::shared_ptr<FourierVisualizer> visualizer_;
    std::shared_ptr<Renderer> renderer_;

    // UI state
    std::vector<std::string> availableImages_;
    int selectedImageIndex_ = 0;
    bool imageLoaded_ = false;
    int imageWidth_ = 0;
    int imageHeight_ = 0;

    // Fourier parameters
    int frequencyCount_;
    int maxFrequencies_;
    std::shared_ptr<ComplexImage> transformedImage_;
    std::shared_ptr<RGBComplexImage> transformedRGBImage_;
    bool useRGB_ = true;  // Default to RGB processing

    // Display options
    bool showOriginal_;
    bool showFourier_;
    bool showFrequencyCircles_ = true;
    bool showPhase_ = false;

    
    // Processing parameters
    int maxImageSize_;
};