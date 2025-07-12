#include "renderer.h"
#include "complex_image.h"
#include "fourier_visualizer.h"
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
    : m_originalTexture(0), m_fourierTexture(0), m_shaderProgram(0),
      m_vao(0), m_vbo(0), m_initialized(false),
      m_showOriginal(true), m_showFourier(true),
      m_showFrequencyCircles(false), m_showPhase(false),
      m_imageWidth(0), m_imageHeight(0) {
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::initializeOpenGL() {
    if (m_initialized) return;
    
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
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vertexShader);
    glAttachShader(m_shaderProgram, fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
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
    
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
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
    
    m_initialized = true;
}

void Renderer::createTextures() {
    if (m_originalTexture == 0) {
        glGenTextures(1, &m_originalTexture);
        glBindTexture(GL_TEXTURE_2D, m_originalTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    
    if (m_fourierTexture == 0) {
        glGenTextures(1, &m_fourierTexture);
        glBindTexture(GL_TEXTURE_2D, m_fourierTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void Renderer::updateTextures() {
    if (!m_image) return;
    
    createTextures();
    
    int width = m_image->getWidth();
    int height = m_image->getHeight();
    
    // Update original image texture
    std::vector<unsigned char> originalData(width * height * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = (y * width + x) * 4;
            auto pixel = m_image->at(x, y);
            unsigned char value = static_cast<unsigned char>(pixel.real() * 255);
            originalData[idx] = value;
            originalData[idx + 1] = value;
            originalData[idx + 2] = value;
            originalData[idx + 3] = 255;
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, m_originalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, originalData.data());
    
    // Update Fourier reconstruction texture
    if (m_reconstructed) {
        std::vector<unsigned char> fourierData(width * height * 4);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = (y * width + x) * 4;
                auto pixel = m_reconstructed->at(x, y);
                unsigned char value = static_cast<unsigned char>(std::clamp(pixel.real() * 255, 0.0, 255.0));
                fourierData[idx] = value;
                fourierData[idx + 1] = value;
                fourierData[idx + 2] = value;
                fourierData[idx + 3] = 255;
            }
        }
        
        glBindTexture(GL_TEXTURE_2D, m_fourierTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fourierData.data());
    }
}

void Renderer::render(int width, int height) {
    if (!m_initialized) {
        initializeOpenGL();
    }
    
    if (!m_image) return;
    
    // Update reconstructed image from visualizer
    if (m_visualizer) {
        try {
            auto reconstructed = m_visualizer->getReconstructedImage();
            m_reconstructed = std::make_shared<ComplexImage>(reconstructed);
        } catch (const std::exception& e) {
            std::cerr << "Error getting reconstructed image: " << e.what() << std::endl;
        }
    }
    
    updateTextures();
    
    // Save current OpenGL state
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    
    // Clear background
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Calculate layout
    float padding = 20.0f;
    float imageAspect = (float)m_imageWidth / m_imageHeight;
    float availableWidth = width - 3 * padding;
    float availableHeight = height - 2 * padding;
    
    float imageWidth, imageHeight;
    int numImages = (m_showOriginal ? 1 : 0) + (m_showFourier ? 1 : 0);
    
    if (numImages > 0) {
        imageWidth = availableWidth / numImages - padding;
        imageHeight = imageWidth / imageAspect;
        
        if (imageHeight > availableHeight) {
            imageHeight = availableHeight;
            imageWidth = imageHeight * imageAspect;
        }
        
        float currentX = padding;
        
        if (m_showOriginal) {
            renderImage(m_originalTexture, currentX, padding, imageWidth, imageHeight);
            currentX += imageWidth + padding;
        }
        
        if (m_showFourier && m_fourierTexture) {
            renderImage(m_fourierTexture, currentX, padding, imageWidth, imageHeight);
        }
    }
    
    // Render frequency visualization lines
    if (m_showFrequencyCircles && m_visualizer) {
        renderFrequencyLines();
    }
    
    // Restore viewport
    glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
}

void Renderer::renderImage(GLuint texture, float x, float y, float width, float height) {
    glUseProgram(m_shaderProgram);
    
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
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::renderFrequencyLines() {
    if (!m_visualizer) return;
    
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
    GLuint colorLoc = glGetUniformLocation(lineShader, "lineColor");
    
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
    m_image = image;
    if (image) {
        m_imageWidth = image->getWidth();
        m_imageHeight = image->getHeight();
    }
}

void Renderer::setVisualizer(std::shared_ptr<FourierVisualizer> visualizer) {
    m_visualizer = visualizer;
}

void Renderer::cleanup() {
    if (m_originalTexture) glDeleteTextures(1, &m_originalTexture);
    if (m_fourierTexture) glDeleteTextures(1, &m_fourierTexture);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
}