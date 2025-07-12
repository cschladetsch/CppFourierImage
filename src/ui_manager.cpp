#include "ui_manager.h"
#include "image_loader.h"
#include "fourier_transform.h"
#include "fourier_visualizer.h"
#include "renderer.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <iostream>
#include <algorithm>

UIManager::UIManager(std::shared_ptr<ImageLoader> imageLoader,
                     std::shared_ptr<FourierTransform> fourierTransform,
                     std::shared_ptr<FourierVisualizer> visualizer,
                     std::shared_ptr<Renderer> renderer)
    : m_imageLoader(imageLoader),
      m_fourierTransform(fourierTransform),
      m_visualizer(visualizer),
      m_renderer(renderer),
      m_frequencyCount(100),
      m_maxFrequencies(1000),
      m_showOriginal(true),
      m_showFourier(true),
      m_animationSpeed(1.0f),
      m_isAnimating(false),
      m_currentFilePath("resources/Demo1.png") {
}

UIManager::~UIManager() {
}

void UIManager::initialize() {
}

void UIManager::update() {
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Image...", "Ctrl+O")) {
                m_showFileDialog = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Handle exit
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Original", nullptr, &m_showOriginal);
            ImGui::MenuItem("Show Fourier", nullptr, &m_showFourier);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Control panel
    ImGui::Begin("Fourier Controls");
    
    // File input
    ImGui::Text("Image File:");
    ImGui::InputText("##filepath", &m_currentFilePath);
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        m_showFileDialog = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Load") && !m_currentFilePath.empty()) {
        loadImage(m_currentFilePath);
    }

    ImGui::Separator();

    // Frequency control
    ImGui::Text("Frequency Components:");
    if (ImGui::SliderInt("##frequencies", &m_frequencyCount, 1, m_maxFrequencies)) {
        updateVisualization();
    }
    
    ImGui::Text("Using %d of %d frequencies", m_frequencyCount, m_maxFrequencies);
    
    // Animation controls
    ImGui::Separator();
    ImGui::Text("Animation:");
    ImGui::Checkbox("Animate", &m_isAnimating);
    if (m_isAnimating) {
        ImGui::SliderFloat("Speed", &m_animationSpeed, 0.1f, 5.0f);
    }

    // Display options
    ImGui::Separator();
    ImGui::Text("Display Options:");
    ImGui::Checkbox("Show Original Image", &m_showOriginal);
    ImGui::Checkbox("Show Fourier Reconstruction", &m_showFourier);
    ImGui::Checkbox("Show Frequency Circles", &m_showFrequencyCircles);
    ImGui::Checkbox("Show Phase Information", &m_showPhase);

    ImGui::End();

    // Status window
    ImGui::Begin("Status");
    if (m_imageLoaded) {
        ImGui::Text("Image: %dx%d", m_imageWidth, m_imageHeight);
        ImGui::Text("Total Frequencies: %d", m_maxFrequencies);
        ImGui::Text("Active Frequencies: %d", m_frequencyCount);
        ImGui::Text("Reconstruction Quality: %.2f%%", 
                   (float)m_frequencyCount / m_maxFrequencies * 100.0f);
    } else {
        ImGui::Text("No image loaded");
    }
    ImGui::End();

    // Handle file dialog
    if (m_showFileDialog) {
        ImGui::Begin("Open Image", &m_showFileDialog);
        // Simple file browser would go here
        // For now, just use the text input
        ImGui::Text("Enter image path:");
        ImGui::InputText("##filebrowser", &m_currentFilePath);
        if (ImGui::Button("Open")) {
            loadImage(m_currentFilePath);
            m_showFileDialog = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            m_showFileDialog = false;
        }
        ImGui::End();
    }

    // Update animation
    if (m_isAnimating && m_imageLoaded) {
        static float animTime = 0.0f;
        animTime += ImGui::GetIO().DeltaTime * m_animationSpeed;
        int animFreq = (int)(animTime * 10.0f) % m_maxFrequencies + 1;
        if (animFreq != m_frequencyCount) {
            m_frequencyCount = animFreq;
            updateVisualization();
        }
    }
}

void UIManager::render() {
    // Rendering is handled by the Renderer class
    if (m_imageLoaded) {
        m_renderer->setShowOriginal(m_showOriginal);
        m_renderer->setShowFourier(m_showFourier);
        m_renderer->setShowFrequencyCircles(m_showFrequencyCircles);
        m_renderer->setShowPhase(m_showPhase);
    }
}

void UIManager::handleInput() {
    // Input is handled by ImGui
}

void UIManager::loadImage(const std::string& filepath) {
    try {
        if (m_imageLoader->loadImage(filepath)) {
            m_currentFilePath = filepath;
            m_imageLoaded = true;
            
            // Get image dimensions
            auto complexImage = m_imageLoader->getComplexImage();
            m_imageWidth = complexImage->getWidth();
            m_imageHeight = complexImage->getHeight();
            
            // Resize large images for better performance
            auto processedImage = complexImage;
            if (m_imageWidth > 512 || m_imageHeight > 512) {
                // Create a smaller version for processing
                size_t newWidth = std::min(static_cast<size_t>(512), static_cast<size_t>(m_imageWidth));
                size_t newHeight = std::min(static_cast<size_t>(512), static_cast<size_t>(m_imageHeight));
                
                auto smallerImage = std::make_shared<ComplexImage>(newWidth, newHeight);
                
                // Simple downsampling
                double scaleX = static_cast<double>(m_imageWidth) / newWidth;
                double scaleY = static_cast<double>(m_imageHeight) / newHeight;
                
                for (size_t y = 0; y < newHeight; ++y) {
                    for (size_t x = 0; x < newWidth; ++x) {
                        size_t srcX = static_cast<size_t>(x * scaleX);
                        size_t srcY = static_cast<size_t>(y * scaleY);
                        smallerImage->at(x, y) = complexImage->at(srcX, srcY);
                    }
                }
                
                processedImage = smallerImage;
                m_imageWidth = newWidth;
                m_imageHeight = newHeight;
                std::cout << "Resized image to " << m_imageWidth << "x" << m_imageHeight << " for better performance." << std::endl;
            }
            
            // Perform Fourier transform
            std::cout << "Performing FFT on " << m_imageWidth << "x" << m_imageHeight << " image..." << std::endl;
            m_transformedImage = std::make_shared<ComplexImage>(m_fourierTransform->transform2D(*processedImage));
            std::cout << "FFT complete." << std::endl;
            
            // Setup visualizer with frequency domain data
            m_visualizer->setImage(*m_transformedImage);
            m_maxFrequencies = std::min(1000, static_cast<int>(m_imageWidth * m_imageHeight / 4));
            m_frequencyCount = std::min(m_frequencyCount, m_maxFrequencies);
            
            // Update visualization
            updateVisualization();
            
            // Setup renderer with original image for display
            m_renderer->setImage(processedImage);
            m_renderer->setVisualizer(m_visualizer);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        m_imageLoaded = false;
    }
}

void UIManager::updateVisualization() {
    if (m_imageLoaded && m_visualizer) {
        m_visualizer->setFrequencyCount(m_frequencyCount);
        m_visualizer->updateAnimation(0.016f); // ~60fps
    }
}