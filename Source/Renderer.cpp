#include "Renderer.hpp"
#include "ComplexImage.hpp"
#include "RgbComplexImage.hpp"
#include "FourierVisualizer.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord);
}
)";

const char* lineVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* lineFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec4 lineColor;

void main() {
    FragColor = lineColor;
}
)";

Renderer::Renderer() 
    : originalTexture_(0), fourierTexture_(0), shaderProgram_(0),
      vao_(0), vbo_(0), initialized_(false),
      showOriginal_(true), showFourier_(true),
      showFrequencyCircles_(false), showPhase_(false),
      imageWidth_(0), imageHeight_(0) {
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initializeOpenGL() {
    if (initialized_) return;
    
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Check for compilation errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }
    
    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }
    
    // Create shader program
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram_, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Setup vertex data
    float vertices[] = {
        // positions   // texture coords
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 0.0f
    };
    
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    initialized_ = true;
}

void Renderer::createTextures() {
    if (originalTexture_ == 0) {
        glGenTextures(1, &originalTexture_);
        glBindTexture(GL_TEXTURE_2D, originalTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    if (fourierTexture_ == 0) {
        glGenTextures(1, &fourierTexture_);
        glBindTexture(GL_TEXTURE_2D, fourierTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void Renderer::updateTextures() {
    if (!image_) return;
    
    createTextures();
    
    int width = image_->getWidth();
    int height = image_->getHeight();
    
    // Update original image texture
    std::vector<unsigned char> originalData(width * height * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            auto pixel = image_->at(x, y);
            unsigned char value = static_cast<unsigned char>(pixel.real() * 255);
            originalData[idx] = value;
            originalData[idx + 1] = value;
            originalData[idx + 2] = value;
            originalData[idx + 3] = 255;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, originalTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, originalData.data());
    
    // Update Fourier reconstruction texture
    if (reconstructed_) {
        std::vector<unsigned char> fourierData(width * height * 4);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 4;
                auto pixel = reconstructed_->at(x, y);
                unsigned char value = static_cast<unsigned char>(std::clamp(pixel.real() * 255, 0.0, 255.0));
                fourierData[idx] = value;
                fourierData[idx + 1] = value;
                fourierData[idx + 2] = value;
                fourierData[idx + 3] = 255;
            }
        }
        
        glBindTexture(GL_TEXTURE_2D, fourierTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fourierData.data());
    }
}

void Renderer::updateRGBTextures() {
    if (!rgbImage_) return;
    
    createTextures();
    
    int width = rgbImage_->getWidth();
    int height = rgbImage_->getHeight();
    
    // Update original RGB image texture
    std::vector<uint32_t> rgbData = rgbImage_->toRGB();
    std::vector<unsigned char> originalData(width * height * 4);
    
    for (int i = 0; i < width * height; ++i) {
        uint32_t pixel = rgbData[i];
        originalData[i * 4] = (pixel >> 24) & 0xFF;     // R
        originalData[i * 4 + 1] = (pixel >> 16) & 0xFF; // G
        originalData[i * 4 + 2] = (pixel >> 8) & 0xFF;  // B
        originalData[i * 4 + 3] = 255;                  // A
    }
    
    glBindTexture(GL_TEXTURE_2D, originalTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, originalData.data());
    
    // Update RGB Fourier reconstruction texture
    if (rgbReconstructed_) {
        std::vector<uint32_t> reconstructedRGB = rgbReconstructed_->toRGB();
        std::vector<unsigned char> fourierData(width * height * 4);
        
        for (int i = 0; i < width * height; ++i) {
            uint32_t pixel = reconstructedRGB[i];
            fourierData[i * 4] = (pixel >> 24) & 0xFF;     // R
            fourierData[i * 4 + 1] = (pixel >> 16) & 0xFF; // G
            fourierData[i * 4 + 2] = (pixel >> 8) & 0xFF;  // B
            fourierData[i * 4 + 3] = 255;                  // A
        }
        
        glBindTexture(GL_TEXTURE_2D, fourierTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fourierData.data());
    }
}

void Renderer::render(int width, int height) {
    if (!initialized_) {
        initializeOpenGL();
    }
    
    if (!image_ && !rgbImage_) return;
    
    // Update reconstructed image from visualizer
    if (visualizer_) {
        try {
            if (isRGB_) {
                auto reconstructed = visualizer_->getReconstructedRGBImage();
                rgbReconstructed_ = std::make_shared<RGBComplexImage>(reconstructed);
            } else {
                auto reconstructed = visualizer_->getReconstructedImage();
                reconstructed_ = std::make_shared<ComplexImage>(reconstructed);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error getting reconstructed image: " << e.what() << std::endl;
        }
    }
    
    if (isRGB_) {
        updateRGBTextures();
    } else {
        updateTextures();
    }
    
    // Save current OpenGL state
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    
    // Clear background
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Calculate layout
    float padding = 20.0f;
    float imageAspect = (float)imageWidth_ / imageHeight_;
    float availableWidth = width - 3 * padding;
    float availableHeight = height - 2 * padding;
    
    float imageWidth, imageHeight;
    int numImages = (showOriginal_ ? 1 : 0) + (showFourier_ ? 1 : 0);
    
    if (numImages > 0) {
        imageWidth = availableWidth / numImages - padding;
        imageHeight = imageWidth / imageAspect;
        
        if (imageHeight > availableHeight) {
            imageHeight = availableHeight;
            imageWidth = imageHeight * imageAspect;
        }
        
        float currentX = padding;
        
        if (showOriginal_) {
            renderImage(originalTexture_, currentX, padding, imageWidth, imageHeight);
            currentX += imageWidth + padding;
        }
        
        if (showFourier_ && fourierTexture_) {
            renderImage(fourierTexture_, currentX, padding, imageWidth, imageHeight);
        }
    }
    
    // Render frequency visualization lines
    if (showFrequencyCircles_ && visualizer_) {
        renderFrequencyLines();
    }
    
    // Restore viewport
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
}

void Renderer::renderImage(GLuint texture, float x, float y, float width, float height) {
    glUseProgram(shaderProgram_);
    
    // Convert to normalized device coordinates
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    float ndcX = (2.0f * x / viewport[2]) - 1.0f;
    float ndcY = 1.0f - (2.0f * y / viewport[3]);
    float ndcW = 2.0f * width / viewport[2];
    float ndcH = 2.0f * height / viewport[3];
    
    // Update vertex data for this image position
    float vertices[] = {
        ndcX,         ndcY - ndcH,  0.0f, 1.0f,
        ndcX + ndcW,  ndcY - ndcH,  1.0f, 1.0f,
        ndcX + ndcW,  ndcY,         1.0f, 0.0f,
        ndcX,         ndcY,         0.0f, 0.0f
    };
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::renderFrequencyLines() {
    if (!visualizer_) return;
    
    // TODO: implement getVisualizationLines in visualizer
    return;
    
    // Create line shader if not exists
    static GLuint lineShader = 0;
    if (lineShader == 0) {
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &lineVertexShaderSource, nullptr);
        glCompileShader(vertexShader);
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &lineFragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);
        
        lineShader = glCreateProgram();
        glAttachShader(lineShader, vertexShader);
        glAttachShader(lineShader, fragmentShader);
        glLinkProgram(lineShader);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    glUseProgram(lineShader);
    
    // Set up projection matrix
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    float projection[16] = {
        2.0f/viewport[2], 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f/viewport[3], 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    GLuint projLoc = glGetUniformLocation(lineShader, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    // Create VAO/VBO for lines
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    
    // Draw each line
    // GLuint colorLoc = glGetUniformLocation(lineShader, "lineColor");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glLineWidth(2.0f); // Not in core profile
    
    // TODO: Draw frequency lines when getVisualizationLines is implemented
    /*
    for (const auto& line : lines) {
        float vertices[] = {
            line.start.x, line.start.y,
            line.end.x, line.end.y
        };
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Set color based on intensity
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, line.intensity);
        glDrawArrays(GL_LINES, 0, 2);
    }
    */
    
    glDisable(GL_BLEND);
    
    // Cleanup
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glUseProgram(0);
}

void Renderer::setImage(std::shared_ptr<ComplexImage> image) {
    image_ = image;
    isRGB_ = false;
    if (image) {
        imageWidth_ = image->getWidth();
        imageHeight_ = image->getHeight();
    }
}

void Renderer::setRGBImage(std::shared_ptr<RGBComplexImage> image) {
    rgbImage_ = image;
    isRGB_ = true;
    if (image) {
        imageWidth_ = image->getWidth();
        imageHeight_ = image->getHeight();
    }
}

void Renderer::setVisualizer(std::shared_ptr<FourierVisualizer> visualizer) {
    visualizer_ = visualizer;
}

void Renderer::cleanup() {
    if (originalTexture_) glDeleteTextures(1, &originalTexture_);
    if (fourierTexture_) glDeleteTextures(1, &fourierTexture_);
    if (shaderProgram_) glDeleteProgram(shaderProgram_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
}