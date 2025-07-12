#pragma once

#include <memory>
#include <string>

class ComplexImage;

class ImageLoader;
class FourierTransform;
class FourierVisualizer;
class Renderer;

class UIManager {
public:
    UIManager(std::shared_ptr<ImageLoader> imageLoader,
              std::shared_ptr<FourierTransform> fourierTransform,
              std::shared_ptr<FourierVisualizer> visualizer,
              std::shared_ptr<Renderer> renderer);
    ~UIManager();

    void initialize();
    void update();
    void render();
    void handleInput();

private:
    void loadImage(const std::string& filepath);
    void updateVisualization();

    // Component references
    std::shared_ptr<ImageLoader> m_imageLoader;
    std::shared_ptr<FourierTransform> m_fourierTransform;
    std::shared_ptr<FourierVisualizer> m_visualizer;
    std::shared_ptr<Renderer> m_renderer;

    // UI state
    std::string m_currentFilePath;
    bool m_showFileDialog = false;
    bool m_imageLoaded = false;
    int m_imageWidth = 0;
    int m_imageHeight = 0;

    // Fourier parameters
    int m_frequencyCount;
    int m_maxFrequencies;
    std::shared_ptr<ComplexImage> m_transformedImage;

    // Display options
    bool m_showOriginal;
    bool m_showFourier;
    bool m_showFrequencyCircles = true;
    bool m_showPhase = false;

    // Animation
    bool m_isAnimating;
    float m_animationSpeed;
};