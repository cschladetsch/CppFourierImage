#include <imgui.h>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <cmath>
#include <ranges>
#include <string_view>
#include <array>
#include <execution>
#include <numeric>

#include "UiManager.hpp"
#include "ImageLoader.hpp"
#include "FourierTransform.hpp"
#include "FourierVisualizer.hpp"
#include "Renderer.hpp"
#include "RgbComplexImage.hpp"

constexpr size_t MaxFreq = 100000uz;

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
      maxFrequencies_(MaxFreq), 
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
    // Show startup popup
    if (showStartupPopup_) {
        ImGui::OpenPopup("Welcome");
        
        // Center the popup
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        
        if (ImGui::BeginPopupModal("Welcome", &showStartupPopup_, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Christian's Visual Thing");
            ImGui::Separator();
            ImGui::Text("Animation will start automatically.");
            ImGui::Text("You can scrub the Frequency slider at any time.");
            ImGui::Spacing();
            
            if (ImGui::Button("OK", ImVec2(120, 0))) {
                showStartupPopup_ = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
    
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
    ImGui::Text("Frequency:");
    
    // Handle animation
    if (isAnimating_ && imageLoaded_) {
        // Update animation time (ImGui frame time is roughly 60fps)
        float deltaTime = ImGui::GetIO().DeltaTime;
        
        if (animationDirection_) {
            animationTime_ += deltaTime;
            if (animationTime_ >= animationDuration_) {
                animationTime_ = animationDuration_;
                animationDirection_ = false;
            }
        } else {
            animationTime_ -= deltaTime;
            if (animationTime_ <= 0.0f) {
                animationTime_ = 0.0f;
                animationDirection_ = true;
            }
        }
    }
    
    // Convert current frequency count to logarithmic scale for slider
    float logMin = std::log10(1.0f);
    auto logMax = std::log10(static_cast<float>(maxFrequencies_));
    auto logValue = std::log10(static_cast<float>(std::max(1uz, frequencyCount_)));
    
    // Override with animation value if animating
    if (isAnimating_ && imageLoaded_) {
        float t = animationTime_ / animationDuration_;
        
        // Apply ease-in-out cubic for smooth acceleration and deceleration
        // This creates slow start, fast middle, slow end
        if (t < 0.5f) {
            t = 4.0f * t * t * t; // ease in cubic
        } else {
            t = 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f; // ease out cubic
        }
        
        logValue = logMin + (logMax - logMin) * t;
        
        // Update frequency count from animation
        size_t newFrequencyCount = static_cast<size_t>(std::pow(10.0f, logValue));
        newFrequencyCount = std::clamp(newFrequencyCount, 1uz, maxFrequencies_);
        
        if (newFrequencyCount != frequencyCount_) {
            frequencyCount_ = newFrequencyCount;
            // Single event dispatch triggers all updates
            EventDispatcher::getInstance().dispatch(
                FrequencyChangeEvent(frequencyCount_, maxFrequencies_)
            );
        }
    }
    
    // Use a float slider for smoother logarithmic control
    if (ImGui::SliderFloat("##logfrequencies", &logValue, logMin, logMax, "")) {
        if (!isAnimating_) {
            // Convert back from logarithmic to linear
            size_t newFrequencyCount = static_cast<size_t>(std::pow(10.0f, logValue));
            newFrequencyCount = std::clamp(newFrequencyCount, 1uz, maxFrequencies_);
            
            if (newFrequencyCount != frequencyCount_) {
                frequencyCount_ = newFrequencyCount;
                // Single event dispatch triggers all updates
                EventDispatcher::getInstance().dispatch(
                    FrequencyChangeEvent(frequencyCount_, maxFrequencies_)
                );
            }
        } else {
            // Stop animation if user manually moves slider
            isAnimating_ = false;
        }
    }
    
    ImGui::Text("Using %zu of %zu frequencies", frequencyCount_, maxFrequencies_);
    
    // Animation controls
    ImGui::Separator();
    if (ImGui::Button(isAnimating_ ? "Stop Animation" : "Animate", ImVec2(-1, 0))) {
        isAnimating_ = !isAnimating_;
        if (isAnimating_) {
            // Calculate starting time based on current position
            float normalizedPos = (logValue - logMin) / (logMax - logMin);
            
            // Inverse of ease-in-out cubic to find time from position
            if (normalizedPos < 0.5f) {
                animationTime_ = std::cbrt(normalizedPos / 4.0f) * animationDuration_;
            } else {
                float temp = 2.0f - 2.0f * normalizedPos;
                animationTime_ = (1.0f - std::cbrt(temp) / 2.0f) * animationDuration_;
            }
            
            // Determine direction based on position
            animationDirection_ = true;
        }
    }
    
    if (isAnimating_) {
        float progress = animationTime_ / animationDuration_;
        if (!animationDirection_) {
            progress = 1.0f - progress;
        }
        ImGui::Text("Animation: %.1f%% (%s)", progress * 100.0f, 
                    animationDirection_ ? "Forward" : "Reverse");
    }

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
                
                // Process channels in parallel
                std::array<int, 3> channels = {0, 1, 2};
                std::for_each(std::execution::par_unseq, channels.begin(), channels.end(),
                    [&](int channel) {
                        // Generate all pixel coordinates
                        std::vector<std::pair<size_t, size_t>> coords;
                        coords.reserve(newWidth * newHeight);
                        
                        for (size_t y = 0; y < newHeight; ++y) {
                            for (size_t x = 0; x < newWidth; ++x) {
                                coords.emplace_back(x, y);
                            }
                        }
                        
                        // Process all pixels in parallel
                        std::for_each(std::execution::par_unseq, coords.begin(), coords.end(),
                            [&](const auto& coord) {
                                auto [x, y] = coord;
                                const auto srcX = static_cast<size_t>(x * scaleX);
                                const auto srcY = static_cast<size_t>(y * scaleY);
                                smallerRGBImage->getChannel(channel).at(x, y) = 
                                    rgbImage->getChannel(channel).at(srcX, srcY);
                            });
                    });
                
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
            
            // Setup renderer with RGB image for display
            renderer_->setRGBImage(processedRGBImage);
            renderer_->setVisualizer(visualizer_);
            
            // Initial visualization setup
            visualizer_->setFrequencyCount(frequencyCount_);
            
            // Compute channel spectrums
            computeChannelSpectrums();
            
            // Dispatch image loaded event
            EventDispatcher::getInstance().dispatch(
                ImageLoadedEvent(imageWidth_, imageHeight_)
            );
            
            // Dispatch initial frequency event
            EventDispatcher::getInstance().dispatch(
                FrequencyChangeEvent(frequencyCount_, maxFrequencies_)
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
        // All other updates happen via events
    }
}

void UIManager::computeChannelSpectrums() {
    if (!transformedRGBImage_ || !visualizer_) return;
    
    // Get the reconstructed RGB image to see which frequencies are active
    auto reconstructedRGB = visualizer_->getReconstructedRGBImage();
    
    size_t width = reconstructedRGB.getWidth();
    size_t height = reconstructedRGB.getHeight();
    size_t centerY = height / 2;
    
    // Process all channels in parallel
    std::array<int, 3> channels = {0, 1, 2};
    
    std::for_each(std::execution::par_unseq, channels.begin(), channels.end(),
        [this, &reconstructedRGB, width, centerY](int channel) {
            const auto& reconstructedChannel = reconstructedRGB.getChannel(channel);
            
            // Clear and resize in one operation
            channelSpectrums_[channel].resize(width / 2);
            
            // Generate x coordinates
            std::vector<size_t> x_coords(width / 2);
            std::iota(x_coords.begin(), x_coords.end(), 0);
            
            // Compute magnitudes in parallel
            std::transform(std::execution::par_unseq, 
                          x_coords.begin(), x_coords.end(),
                          channelSpectrums_[channel].begin(),
                          [&reconstructedChannel, centerY](size_t x) {
                              auto reconstructedValue = reconstructedChannel.at(x, centerY);
                              Scalar magnitude = std::abs(reconstructedValue);
                              // Apply log scale for better visualization
                              return static_cast<float>(std::log10(1.0 + magnitude));
                          });
            
            // Find max value for normalization
            auto max_it = std::max_element(std::execution::par_unseq,
                                         channelSpectrums_[channel].begin(), 
                                         channelSpectrums_[channel].end());
            float maxVal = *max_it;
            
            if (maxVal > 0) {
                // Normalize in parallel
                std::for_each(std::execution::par_unseq,
                            channelSpectrums_[channel].begin(),
                            channelSpectrums_[channel].end(),
                            [maxVal](float& val) { val /= maxVal; });
            }
            
            // Apply smoothing using a simple moving average
            constexpr int smoothingWindow = 7;
            std::vector<float> smoothed(channelSpectrums_[channel].size());
            
            // Use parallel execution for smoothing
            std::vector<size_t> indices(channelSpectrums_[channel].size());
            std::iota(indices.begin(), indices.end(), 0);
            
            std::transform(std::execution::par_unseq,
                          indices.begin(), indices.end(),
                          smoothed.begin(),
                          [this, channel](size_t i) {
                              float sum = 0.0f;
                              int count = 0;
                              
                              // Average over neighboring points
                              for (int j = -smoothingWindow/2; j <= smoothingWindow/2; ++j) {
                                  int idx = static_cast<int>(i) + j;
                                  if (idx >= 0 && idx < static_cast<int>(channelSpectrums_[channel].size())) {
                                      sum += channelSpectrums_[channel][idx];
                                      count++;
                                  }
                              }
                              
                              return sum / count;
                          });
            
            channelSpectrums_[channel] = std::move(smoothed);
        });
}

void UIManager::renderSpectrumWindow() {
    // Set window transparency - both background and content
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.8f);  // 80% opacity = 20% transparent
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.8f)); // Window background also 80% opacity
    
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
        
        // Draw background with transparency
        draw_list->AddRectFilled(canvas_pos, 
            ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), 
            IM_COL32(50, 50, 50, 204)); // 80% opacity (204/255)
        
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
                    
                    draw_list->AddLine(ImVec2(x0, y0), ImVec2(x1, y1), col, 3.0f);
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
    
    // Restore original styles
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}
