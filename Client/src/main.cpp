#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include <iostream>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui.h"
#include "rendering/Renderer.h"
#include "Application.h"

const int WINDOW_WIDTH = 2000;
const int WINDOW_HEIGHT = 1000;

#ifndef __EMSCRIPTEN__
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*) {
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return; // ignore noise
    std::cout << "GL CALLBACK: "
              << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR ** " : "")
              << "type=" << std::hex << type
              << " severity=" << severity
              << " id=" << id
              << " msg=" << message << std::dec << std::endl;
    if (type == GL_DEBUG_TYPE_ERROR) std::abort();
}
#endif

// Global state needed for Emscripten's main loop
struct AppState {
    GLFWwindow* window;
    Renderer* renderer;
    Application* app;
};

AppState* g_appState = nullptr;

void main_loop() {
    g_appState->renderer->Clear();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Update and render application
    g_appState->app->OnUpdate(0.0f);
    g_appState->app->OnRender();
    g_appState->app->OnImGuiRender();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(g_appState->window);
    glfwPollEvents();
}

int main() {
    if (!glfwInit()) return -1;

#ifdef __EMSCRIPTEN__
    // WebGL 2.0 = OpenGL ES 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // Desktop OpenGL 4.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "3D Tic-Tac-Toe", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

#ifndef __EMSCRIPTEN__
    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR: GLEW NOT OK" << std::endl;
        return -1;
    }

    // Enable debug output AFTER context and GLEW are ready
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    std::cout << glGetString(GL_VERSION) << std::endl;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Renderer renderer;

    // ImGui setup
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui::StyleColorsDark();
#ifdef __EMSCRIPTEN__
    ImGui_ImplOpenGL3_Init("#version 300 es");
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif

    // Create application
    Application app;

    // Setup global state for main loop
    AppState appState;
    appState.window = window;
    appState.renderer = &renderer;
    appState.app = &app;
    g_appState = &appState;

#ifdef __EMSCRIPTEN__
    // Emscripten: Use callback-based main loop
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    // Windows: Use traditional while loop
    while (!glfwWindowShouldClose(window)) {
        main_loop();
    }
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    
    return 0;
}