#include <iostream>
#include <memory>
#include <filesystem>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "ImageLoader.hpp"
#include "FourierTransform.hpp"
#include "FourierVisualizer.hpp"
#include "Renderer.hpp"
#include "UiManager.hpp"

int main(int argc, char* argv[]) {
    // Parse command line arguments
    size_t maxImageSize = 512;  // Default size
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--size" || arg == "-s") && i + 1 < argc) {
            int temp_size = std::atoi(argv[++i]);
            if (temp_size <= 0 || temp_size > 2048) {
                std::cerr << "Invalid size. Using default 512. Valid range: 1-2048\n";
                maxImageSize = 512;
            } else {
                maxImageSize = static_cast<size_t>(temp_size);
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -s, --size <N>    Set maximum image size for processing (default: 512)\n";
            std::cout << "  -h, --help        Show this help message\n";
            return 0;
        }
    }
    
    std::cout << "Maximum image processing size: " << maxImageSize << "x" << maxImageSize << "\n";
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Fourier Image Analyzer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    std::cout << "Window created successfully" << std::endl;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }
    std::cout << "GLAD initialized successfully" << std::endl;

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize our components
    auto imageLoader = std::make_shared<ImageLoader>();
    auto fourierTransform = std::make_shared<FourierTransform>();
    auto visualizer = std::make_shared<FourierVisualizer>();
    auto renderer = std::make_shared<Renderer>();
    auto uiManager = std::make_unique<UIManager>(imageLoader, fourierTransform, visualizer, renderer, maxImageSize);
    
    // Initialize UI Manager (auto-loads default image)
    std::cout << "Initializing UI Manager..." << std::endl;
    uiManager->initialize();
    std::cout << "UI Manager initialized." << std::endl;

    // Clear color
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    std::cout << "Starting main loop..." << std::endl;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Update UI
        uiManager->update();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, 
                     clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render our custom content
        renderer->render(display_w, display_h);

        // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}