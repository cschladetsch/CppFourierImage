#include "UiManager.hpp"
#include "ImageLoader.hpp"
#include "FourierTransform.hpp"
#include "FourierVisualizer.hpp"
#include "Renderer.hpp"
#include "RgbComplexImage.hpp"
#include <imgui.h>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cmath>

UIManager::UIManager(std::shared_ptr<ImageLoader> imageLoader,
                     std::shared_ptr<FourierTransform> fourierTransform,
                     std::shared_ptr<FourierVisualizer> visualizer,
                     std::shared_ptr<Renderer> renderer,
                     int maxImageSize)
    : imageLoader_(imageLoader),
      fourierTransform_(fourierTransform),
      visualizer_(visualizer),
      renderer_(renderer),
      frequencyCount_(100),
      maxFrequencies_(50000),
      showOriginal_(true),
      showFourier_(true),
      maxImageSize_(maxImageSize) {
}

UIManager::~UIManager() {
}

void UIManager::initialize() {
    // Scan for available images in Resources folder
    scanResourcesFolder();
    
    // Auto-load the first image if available
    if (!availableImages_.empty()) {
        loadImage(availableImages_[0]);
    }
}

void UIManager::scanResourcesFolder() {
    availableImages_.clear();
    
    const std::string resourcesPath = "./Resources/";
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(resourcesPath)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
                
                // Check if it's a supported image format
                if (extension == ".jpg" || extension == ".jpeg" || 
                    extension == ".png" || extension == ".bmp" ||
                    extension == ".gif" || extension == ".tiff" ||
                    extension == ".tif") {
                    availableImages_.push_back(entry.path().string());
                }
            }
        }
        
        // Sort the files for consistent ordering
        std::sort(availableImages_.begin(), availableImages_.end());
    } catch (const std::exception& e) {
        std::cerr << "Error scanning resources folder: " << e.what() << std::endl;
    }
}

void UIManager::update() {
    // Main control panel
    ImGui::Begin("Fourier Transform Controls");
    
    // Image selector
    if (!availableImages_.empty()) {
        ImGui::Text("Select Image:");
        ImGui::Separator();
        
        // Radio buttons for image selection
        for (size_t i = 0; i < availableImages_.size(); ++i) {
            std::string filename = std::filesystem::path(availableImages_[i]).filename().string();
            bool isSelected = (static_cast<int>(i) == selectedImageIndex_);
            
            if (ImGui::RadioButton(filename.c_str(), isSelected)) {
                if (static_cast<int>(i) != selectedImageIndex_) {
                    selectedImageIndex_ = static_cast<int>(i);
                    loadImage(availableImages_[selectedImageIndex_]);
                }
            }
        }
        
        ImGui::Separator();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No images found in Resources folder!");
    }

    // Frequency control with logarithmic scale
    ImGui::Text("Frequency Components:");
    
    // Convert current frequency count to logarithmic scale for slider
    float logMin = std::log10(1.0f);
    float logMax = std::log10(static_cast<float>(maxFrequencies_));
    float logValue = std::log10(static_cast<float>(std::max(1, frequencyCount_)));
    
    // Use a float slider for smoother logarithmic control
    if (ImGui::SliderFloat("##logfrequencies", &logValue, logMin, logMax, "")) {
        // Convert back from logarithmic to linear
        frequencyCount_ = static_cast<int>(std::pow(10.0f, logValue));
        frequencyCount_ = std::clamp(frequencyCount_, 1, maxFrequencies_);
        updateVisualization();
    }
    
    ImGui::Text("Using %d of %d frequencies", frequencyCount_, maxFrequencies_);

    // Display options
    ImGui::Separator();
    ImGui::Text("Display Options:");
    if (ImGui::Checkbox("Show Original Image", &showOriginal_)) {
        renderer_->setShowOriginal(showOriginal_);
    }
    if (ImGui::Checkbox("Show Fourier Reconstruction", &showFourier_)) {
        renderer_->setShowFourier(showFourier_);
    }
    if (ImGui::Checkbox("Show Frequency Circles", &showFrequencyCircles_)) {
        renderer_->setShowFrequencyCircles(showFrequencyCircles_);
    }
    if (ImGui::Checkbox("Show Phase Information", &showPhase_)) {
        renderer_->setShowPhase(showPhase_);
    }

    ImGui::End();

    // Status window
    ImGui::Begin("Status");
    if (imageLoaded_) {
        ImGui::Text("Image: %dx%d", imageWidth_, imageHeight_);
        ImGui::Text("Total Frequencies: %d", maxFrequencies_);
        ImGui::Text("Active Frequencies: %d", frequencyCount_);
        ImGui::Text("Reconstruction Quality: %.2f%%", 
                   (float)frequencyCount_ / maxFrequencies_ * 100.0f);
    } else {
        ImGui::Text("No image loaded");
    }
    ImGui::End();
}

void UIManager::render() {
    // Rendering is handled by the Renderer class
}

void UIManager::handleInput() {
    // Input is handled by ImGui
}

void UIManager::loadImage(const std::string& filepath) {
    try {
        if (imageLoader_->loadImage(filepath)) {
            imageLoaded_ = true;
            
            // Get RGB image
            auto rgbImage = imageLoader_->getRGBComplexImage();
            if (!rgbImage) {
                std::cerr << "Failed to get RGB image" << std::endl;
                imageLoaded_ = false;
                return;
            }
            
            imageWidth_ = rgbImage->getWidth();
            imageHeight_ = rgbImage->getHeight();
            
            // Resize large images for better performance
            auto processedRGBImage = rgbImage;
            if (imageWidth_ > maxImageSize_ || imageHeight_ > maxImageSize_) {
                // Create a smaller version for processing
                size_t newWidth = std::min(static_cast<size_t>(maxImageSize_), static_cast<size_t>(imageWidth_));
                size_t newHeight = std::min(static_cast<size_t>(maxImageSize_), static_cast<size_t>(imageHeight_));
                
                auto smallerRGBImage = std::make_shared<RGBComplexImage>(newWidth, newHeight);
                
                // Simple downsampling for each channel
                double scaleX = static_cast<double>(imageWidth_) / newWidth;
                double scaleY = static_cast<double>(imageHeight_) / newHeight;
                
                for (int channel = 0; channel < 3; ++channel) {
                    for (size_t y = 0; y < newHeight; ++y) {
                        for (size_t x = 0; x < newWidth; ++x) {
                            size_t srcX = static_cast<size_t>(x * scaleX);
                            size_t srcY = static_cast<size_t>(y * scaleY);
                            smallerRGBImage->getChannel(channel).at(x, y) = rgbImage->getChannel(channel).at(srcX, srcY);
                        }
                    }
                }
                
                processedRGBImage = smallerRGBImage;
                imageWidth_ = newWidth;
                imageHeight_ = newHeight;
                std::cout << "Resized image to " << imageWidth_ << "x" << imageHeight_ << " for better performance." << std::endl;
            }
            
            // Perform RGB Fourier transform
            std::cout << "Performing RGB FFT on " << imageWidth_ << "x" << imageHeight_ << " image..." << std::endl;
            transformedRGBImage_ = std::make_shared<RGBComplexImage>(fourierTransform_->transformRGB2D(*processedRGBImage));
            std::cout << "RGB FFT complete." << std::endl;
            
            // Setup visualizer with RGB frequency domain data
            visualizer_->setRGBImage(*transformedRGBImage_);
            maxFrequencies_ = std::min(50000, static_cast<int>(imageWidth_ * imageHeight_ / 4));
            frequencyCount_ = std::min(frequencyCount_, maxFrequencies_);
            
            // Update visualization
            updateVisualization();
            
            // Setup renderer with RGB image for display
            renderer_->setRGBImage(processedRGBImage);
            renderer_->setVisualizer(visualizer_);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading image: " << e.what() << std::endl;
        imageLoaded_ = false;
    }
}

void UIManager::updateVisualization() {
    if (imageLoaded_ && visualizer_) {
        visualizer_->setFrequencyCount(frequencyCount_);
        visualizer_->updateAnimation(0.016f); // ~60fps
    }
}