#include <gtest/gtest.h>
#include "fourier_visualizer.h"
#include "complex_image.h"
#include <memory>

class FourierVisualizerTest : public ::testing::Test {
protected:
    void SetUp() override {
        visualizer = std::make_shared<FourierVisualizer>();
        
        // Create a simple test image
        test_image = std::make_unique<ComplexImage>(8, 8);
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                double value = (x + y) % 2 ? 1.0 : 0.0;
                test_image->at(x, y) = std::complex<double>(value, 0);
            }
        }
    }
    
    std::shared_ptr<FourierVisualizer> visualizer;
    std::unique_ptr<ComplexImage> test_image;
};

TEST_F(FourierVisualizerTest, SetImageAndFrequencyCount) {
    visualizer->setImage(*test_image);
    visualizer->setFrequencyCount(10);
    
    // Test passes if no exceptions are thrown
    SUCCEED();
}

TEST_F(FourierVisualizerTest, FrequencyCountBounds) {
    visualizer->setImage(*test_image);
    
    // Test minimum bound
    visualizer->setFrequencyCount(1);
    SUCCEED();
    
    // Test maximum bound  
    visualizer->setFrequencyCount(64); // 8x8 image
    SUCCEED();
}

TEST_F(FourierVisualizerTest, UpdateAnimationWithoutImage) {
    // Should not crash when no image is set
    EXPECT_NO_THROW(visualizer->updateAnimation(0.016f));
}

TEST_F(FourierVisualizerTest, GetAnimationState) {
    visualizer->setImage(*test_image);
    visualizer->setFrequencyCount(5);
    
    auto& state = visualizer->getAnimationState();
    
    // Should have animation state
    EXPECT_EQ(state.current_frequency_count, 5);
}

TEST_F(FourierVisualizerTest, AnimationUpdates) {
    visualizer->setImage(*test_image);
    visualizer->setFrequencyCount(10);
    
    // Enable animation
    auto& state = visualizer->getAnimationState();
    state.is_animating = true;
    
    float initial_time = state.time_accumulator;
    
    // Update animation
    visualizer->updateAnimation(0.1f);
    
    // Time should have accumulated
    EXPECT_GT(state.time_accumulator, initial_time);
}

TEST_F(FourierVisualizerTest, GetReconstructedImage) {
    visualizer->setImage(*test_image);
    visualizer->setFrequencyCount(10);
    visualizer->updateAnimation(0.0f);
    
    auto reconstructed = visualizer->getReconstructedImage();
    
    // Should have same dimensions as input
    EXPECT_EQ(reconstructed.getWidth(), test_image->getWidth());
    EXPECT_EQ(reconstructed.getHeight(), test_image->getHeight());
}

TEST_F(FourierVisualizerTest, GetMagnitudeSpectrum) {
    visualizer->setImage(*test_image);
    
    auto magnitude = visualizer->getMagnitudeSpectrum();
    
    // Should have values for each pixel
    EXPECT_EQ(magnitude.size(), test_image->getWidth() * test_image->getHeight());
    
    // All magnitudes should be non-negative
    for (auto mag : magnitude) {
        EXPECT_GE(mag, 0.0f);
    }
}

TEST_F(FourierVisualizerTest, GetPhaseSpectrum) {
    visualizer->setImage(*test_image);
    
    auto phase = visualizer->getPhaseSpectrum();
    
    // Should have values for each pixel
    EXPECT_EQ(phase.size(), test_image->getWidth() * test_image->getHeight());
    
    // All phases should be in [-pi, pi]
    for (auto ph : phase) {
        EXPECT_GE(ph, -M_PI);
        EXPECT_LE(ph, M_PI);
    }
}

TEST_F(FourierVisualizerTest, HandleEmptyImage) {
    ComplexImage empty_image(0, 0);
    
    // Should handle empty image gracefully
    EXPECT_NO_THROW(visualizer->setImage(empty_image));
    EXPECT_NO_THROW(visualizer->setFrequencyCount(10));
    EXPECT_NO_THROW(visualizer->updateAnimation(0.016f));
    
    auto magnitude = visualizer->getMagnitudeSpectrum();
    EXPECT_EQ(magnitude.size(), 0);
}