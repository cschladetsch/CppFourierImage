#pragma once

#include <memory>
#include <glad/glad.h>

class ComplexImage;
class FourierVisualizer;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void render(int width, int height);
    
    void setImage(std::shared_ptr<ComplexImage> image);
    void setVisualizer(std::shared_ptr<FourierVisualizer> visualizer);
    
    void setShowOriginal(bool show) { m_showOriginal = show; }
    void setShowFourier(bool show) { m_showFourier = show; }
    void setShowFrequencyCircles(bool show) { m_showFrequencyCircles = show; }
    void setShowPhase(bool show) { m_showPhase = show; }
    
private:
    void initializeOpenGL();
    void createTextures();
    void updateTextures();
    void renderImage(GLuint texture, float x, float y, float width, float height);
    void renderFrequencyLines();
    void cleanup();
    
    std::shared_ptr<ComplexImage> m_image;
    std::shared_ptr<ComplexImage> m_reconstructed;
    std::shared_ptr<FourierVisualizer> m_visualizer;
    
    GLuint m_originalTexture;
    GLuint m_fourierTexture;
    GLuint m_shaderProgram;
    GLuint m_vao;
    GLuint m_vbo;
    
    bool m_initialized;
    bool m_showOriginal;
    bool m_showFourier;
    bool m_showFrequencyCircles;
    bool m_showPhase;
    
    int m_imageWidth;
    int m_imageHeight;
};