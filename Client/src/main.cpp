#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui.h"
#include "rendering/Renderer.h"
#include "Application.h"

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 1000;

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

struct AppState {
    GLFWwindow* window;
    Application* app;
};

AppState* g_appState = nullptr;
static Application* g_App = nullptr;

void main_loop() {
	Renderer::Clear();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    g_appState->app->OnUpdate(0.0f);
    g_appState->app->OnRender();
    g_appState->app->OnImGuiRender();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(g_appState->window);
    glfwPollEvents();
}

void FramebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    if (g_App)
    {
        g_App->OnResize(width, height);
    }
}

int main() {
    if (!glfwInit()) return -1;

    // Desktop OpenGL 4.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "3D Tic-Tac-Toe", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR: GLEW NOT OK" << std::endl;
        return -1;
    }

    // Enable debug output
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    // ImGui 
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");

    // App
    Application app(WINDOW_WIDTH, WINDOW_HEIGHT);
    g_App = &app;

    AppState appState;
    appState.window = window;
    appState.app = &app;
    g_appState = &appState;

    while (!glfwWindowShouldClose(window)) {
        main_loop();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    
    return 0;
}