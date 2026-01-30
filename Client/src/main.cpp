#ifdef __EMSCRIPTEN__
    #include <GLES3/gl3.h>
    #include <GLFW/glfw3.h>
    #include <emscripten/emscripten.h>
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

// inital window size
const int WINDOW_WIDTH  = 2000;
const int WINDOW_HEIGHT = 1000;

#ifndef __EMSCRIPTEN__
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei,
                                const GLchar* message, const void*)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return; // ignore small warnings
    std::cout << "GL CALLBACK: "
              << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR ** " : "")
              << "type=" << std::hex << type
              << " severity=" << severity
              << " id=" << id
              << " msg=" << message << std::dec << std::endl;
    if (type == GL_DEBUG_TYPE_ERROR) std::abort();
}
#endif

struct AppState
{
    GLFWwindow* window;
    Application* app;
};

static AppState* g_appState = nullptr;
static Application* g_App      = nullptr;

static void main_loop()
{
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

// Window size changes
static void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    if (g_App)
    {
        g_App->OnResize(width, height);
    }
}

int main()
{
    if (!glfwInit())
        return -1;

#ifdef __EMSCRIPTEN__
    // WebGL2 / ES3
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    // Desktop 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "3D Tic-Tac-Toe", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

#ifndef __EMSCRIPTEN__
    if (glewInit() != GLEW_OK)
    {
        std::cout << "ERROR: GLEW NOT OK" << std::endl;
        return -1;
    }

    // Enable OpenGL debugging output
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(MessageCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) << std::endl;

    // Blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // ImGui
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
	// Setup application
    Application app(WINDOW_WIDTH, WINDOW_HEIGHT);
    g_App = &app;

    AppState appState;
    appState.window = window;
    appState.app    = &app;
    g_appState      = &appState;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (!glfwWindowShouldClose(window))
    {
        main_loop();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
#endif

    return 0;
}