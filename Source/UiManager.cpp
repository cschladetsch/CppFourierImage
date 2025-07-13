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
#include <ranges>
#include <string_view>
#include <array>
#include <execution>

UIManager::UIManager(std::shared_ptr<ImageLoader> imageLoader,
                     std::shared_ptr<FourierTransform> fourierTransform,
                     std::shared_ptr<FourierVisualizer> visualizer,
                     std::shared_ptr<Renderer> renderer,
                     size_t maxImageSize)
    : imageLoader_(imageLoader),
      fourierTransform_(fourierTransform),
      visualizer_(visualizer),
      renderer_(renderer),
      frequencyCount_(100uz),
      maxFrequencies_(50000uz),
      maxImageSize_(maxImageSize) {
    
    // Subscribe to frequency change events
    frequencyChangeHandlerId_ = EventDispatcher::getInstance().subscribe<FrequencyChangeEvent>(
        [this](const FrequencyChangeEvent&) {
            // Update spectrums when frequency changes
            if (imageLoaded_ && transformedRGBImage_) {
                computeChannelSpectrums();
            }
        }
    );
}

UIManager::~UIManager() {
    // Unsubscribe from events
    EventDispatcher::getInstance().unsubscribe<FrequencyChangeEvent>(frequencyChangeHandlerId_);
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
    
    constexpr std::string_view resourcesPath = "./Resources/";
    
    // Define supported extensions using a constexpr array
    constexpr std::array<std::string_view, 6> supportedExtensions{
        ".jpg", ".jpeg", ".png", ".bmp", 
        ".tiff", ".tif"
    };
    
    try {
        // Use C++23 ranges to filter and process directory entries
        auto entries = std::filesystem::directory_iterator(resourcesPath);
        
        std::vector<std::string> imageFiles;
        
        for (const auto& entry : entries) {
            if (entry.is_regular_file()) {
                auto path = entry.path();
                auto extension = path.extension().string();
                std::ranges::transform(extension, extension.begin(), ::tolower);
                
                if (std::ranges::find(supportedExtensions, extension) != supportedExtensions.end()) {
                    imageFiles.push_back(path.string());
                }
            }
        }
        
        // Sort for consistent ordering
        std::ranges::sort(imageFiles);
        availableImages_ = std::move(imageFiles);
        
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
        
        // Radio buttons for image selection using C++23 ranges
        auto imageIndices = std::views::iota(0uz, availableImages_.size());
        
        std::ranges::for_each(imageIndices, [this](size_t i) {
            const auto filename = std::filesystem::path(availableImages_[i]).filename().string();
            const bool isSelected = (i == selectedImageIndex_);
            
            if (ImGui::RadioButton(filename.c_str(), isSelected)) {
                if (i != selectedImageIndex_) {
                    selectedImageIndex_ = i;
                    loadImage(availableImages_[selectedImageIndex_]);
                }
            }
        });
        
        ImGui::Separator();
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No images found in Resources folder!");
    }

    // Frequency control with logarithmic scale
    ImGui::Text("Frequency Components:");
    
    // Convert current frequency count to logarithmic scale for slider
    float logMin = std::log10(1.0f);
    auto logMax = std::log10(static_cast<float>(maxFrequencies_));
    auto logValue = std::log10(static_cast<float>(std::max(1uz, frequencyCount_)));
    
    // Use a float slider for smoother logarithmic control
    if (ImGui::SliderFloat("##logfrequencies", &logValue, logMin, logMax, "")) {
        // Convert back from logarithmic to linear
        frequencyCount_ = static_cast<size_t>(std::pow(10.0f, logValue));
        frequencyCount_ = std::clamp(frequencyCount_, 1uz, maxFrequencies_);
        updateVisualization();
        
        // Dispatch frequency change event
        EventDispatcher::getInstance().dispatch(
            FrequencyChangeEvent(frequencyCount_, maxFrequencies_)
        );
    }
    
    ImGui::Text("Using %zu of %zu frequencies", frequencyCount_, maxFrequencies_);


    ImGui::End();

    // Status window
    ImGui::Begin("Status");
    if (imageLoaded_) {
        ImGui::Text("Image: %zux%zu", imageWidth_, imageHeight_);
        ImGui::Text("Total Frequencies: %zu", maxFrequencies_);
        ImGui::Text("Active Frequencies: %zu", frequencyCount_);
        ImGui::Text("Reconstruction Quality: %.2f%%", 
                   static_cast<float>(frequencyCount_) / static_cast<float>(maxFrequencies_) * 100.0f);
    } else {
        ImGui::Text("No image loaded");
    }
    ImGui::End();
    
    // Render spectrum window
    if (showSpectrumWindow_ && imageLoaded_) {
        renderSpectrumWindow();
    }
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
                auto newWidth = std::min(maxImageSize_, imageWidth_);
                auto newHeight = std::min(maxImageSize_, imageHeight_);
                
                auto smallerRGBImage = std::make_shared<RGBComplexImage>(newWidth, newHeight);
                
                // Simple downsampling for each channel using C++23 ranges
                const double scaleX = static_cast<double>(imageWidth_) / static_cast<double>(newWidth);
                const double scaleY = static_cast<double>(imageHeight_) / static_cast<double>(newHeight);
                
                // Process channels
                for (int channel = 0; channel < 3; ++channel) {
                    // Process each pixel
                    for (size_t y = 0; y < newHeight; ++y) {
                        for (size_t x = 0; x < newWidth; ++x) {
                            const auto srcX = static_cast<size_t>(x * scaleX);
                            const auto srcY = static_cast<size_t>(y * scaleY);
                            smallerRGBImage->getChannel(channel).at(x, y) = 
                                rgbImage->getChannel(channel).at(srcX, srcY);
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
            maxFrequencies_ = std::min(50000uz, imageWidth_ * imageHeight_ / 4);
            frequencyCount_ = std::min(frequencyCount_, maxFrequencies_);
            
            // Update visualization
            updateVisualization();
            
            // Setup renderer with RGB image for display
            renderer_->setRGBImage(processedRGBImage);
            renderer_->setVisualizer(visualizer_);
            
            // Compute channel spectrums
            computeChannelSpectrums();
            
            // Dispatch image loaded event
            EventDispatcher::getInstance().dispatch(
                ImageLoadedEvent(imageWidth_, imageHeight_)
            );
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

void UIManager::computeChannelSpectrums() {
    if (!transformedRGBImage_ || !visualizer_) return;
    
    // Get the reconstructed RGB image to see which frequencies are active
    auto reconstructedRGB = visualizer_->getReconstructedRGBImage();
    
    size_t width = reconstructedRGB.getWidth();
    size_t height = reconstructedRGB.getHeight();
    size_t centerY = height / 2;
    
    // Compute spectrum for each channel
    for (int channel = 0; channel < 3; ++channel) {
        const auto& reconstructedChannel = reconstructedRGB.getChannel(channel);
        
        // Clear and reserve space
        channelSpectrums_[channel].clear();
        channelSpectrums_[channel].reserve(width / 2);
        
        // Take a horizontal slice through the center of the frequency domain
        for (size_t x = 0; x < width / 2; ++x) {
            auto reconstructedValue = reconstructedChannel.at(x, centerY);
            Scalar magnitude = std::abs(reconstructedValue);
            
            // Apply log scale for better visualization
            Scalar logMag = std::log10(1.0 + magnitude);
            channelSpectrums_[channel].push_back(static_cast<float>(logMag));
        }
        
        // Normalize the spectrum
        float maxVal = *std::max_element(channelSpectrums_[channel].begin(), 
                                       channelSpectrums_[channel].end());
        if (maxVal > 0) {
            for (auto& val : channelSpectrums_[channel]) {
                val /= maxVal;
            }
        }
    }
}

void UIManager::renderSpectrumWindow() {
    ImGui::Begin("RGB Frequency Spectrum", &showSpectrumWindow_);
    
    if (channelSpectrums_[0].empty()) {
        ImGui::Text("No spectrum data available");
    } else {
        ImGui::Text("Frequency Spectrum (All Channels)");
        ImGui::Separator();
        
        // Define colors for each channel
        const ImVec4 colors[3] = {
            ImVec4(1.0f, 0.2f, 0.2f, 1.0f),  // Red
            ImVec4(0.2f, 1.0f, 0.2f, 1.0f),  // Green
            ImVec4(0.2f, 0.2f, 1.0f, 1.0f)   // Blue
        };
        
        const char* labels[3] = { "Red", "Green", "Blue" };
        
        // Use ImGui's low-level drawing API to overlay all curves
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size(500, 200);
        
        // Draw background
        draw_list->AddRectFilled(canvas_pos, 
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), 
            IM_COL32(50, 50, 50, 255));
        
        // Draw grid lines
        for (int i = 0; i <= 4; ++i) {
            float y = canvas_pos.y + (i * canvas_size.y / 4);
            draw_list->AddLine(
                ImVec2(canvas_pos.x, y),
                ImVec2(canvas_pos.x + canvas_size.x, y),
                IM_COL32(100, 100, 100, 100));
        }
        
        // Plot all three curves on the same axes
        if (!channelSpectrums_[0].empty()) {
            int data_count = static_cast<int>(channelSpectrums_[0].size());
            
            for (int channel = 0; channel < 3; ++channel) {
                ImU32 col = ImGui::ColorConvertFloat4ToU32(colors[channel]);
                
                for (int i = 1; i < data_count; ++i) {
                    float x0 = canvas_pos.x + (static_cast<float>(i-1) / data_count) * canvas_size.x;
                    float x1 = canvas_pos.x + (static_cast<float>(i) / data_count) * canvas_size.x;
                    float y0 = canvas_pos.y + (1.0f - channelSpectrums_[channel][i-1]) * canvas_size.y;
                    float y1 = canvas_pos.y + (1.0f - channelSpectrums_[channel][i]) * canvas_size.y;
                    
                    draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), col, 2.0f);
                }
            }
        }
        
        // Add legend
        ImGui::Dummy(canvas_size);
        for (int i = 0; i < 3; ++i) {
            ImGui::SameLine();
            ImGui::TextColored(colors[i], "%s", labels[i]);
        }
        
        ImGui::Separator();
        ImGui::Text("Frequencies: 0 - %zu", channelSpectrums_[0].size());
        ImGui::Text("Active frequencies: %zu / %zu", frequencyCount_, maxFrequencies_);
        ImGui::Text("Log scale applied for visualization");
    }
    
    ImGui::End();
}