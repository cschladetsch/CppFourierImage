#pragma once

#include <memory>
#include <glad/glad.h>

class ComplexImage;
class RGBComplexImage;
class FourierVisualizer;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void render(int width, int height);
    
    void setImage(std::shared_ptr<ComplexImage> image);
    void setRGBImage(std::shared_ptr<RGBComplexImage> image);
    void setVisualizer(std::shared_ptr<FourierVisualizer> visualizer);
    
    void setShowOriginal(bool show) { showOriginal_ = show; }
    void setShowFourier(bool show) { showFourier_ = show; }
    void setShowFrequencyCircles(bool show) { showFrequencyCircles_ = show; }
    void setShowPhase(bool show) { showPhase_ = show; }
    
private:
    void initializeOpenGL();
    void createTextures();
    void updateTextures();
    void updateRGBTextures();
    void renderImage(GLuint texture, float x, float y, float width, float height);
    void renderFrequencyLines();
    void cleanup();
    
    std::shared_ptr<ComplexImage> image_;
    std::shared_ptr<RGBComplexImage> rgbImage_;
    std::shared_ptr<ComplexImage> reconstructed_;
    std::shared_ptr<RGBComplexImage> rgbReconstructed_;
    std::shared_ptr<FourierVisualizer> visualizer_;
    bool isRGB_ = false;
    
    GLuint originalTexture_;
    GLuint fourierTexture_;
    GLuint shaderProgram_;
    GLuint vao_;
    GLuint vbo_;
    
    bool initialized_;
    bool showOriginal_;
    bool showFourier_;
    bool showFrequencyCircles_;
    bool showPhase_;
    
    int imageWidth_;
    int imageHeight_;
};