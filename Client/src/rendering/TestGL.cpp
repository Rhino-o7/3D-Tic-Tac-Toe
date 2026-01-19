#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui.h"
#include "Renderer.h"



#include "tests/TestClearColor.h"
#include "tests/TestTexture.h"
#include "tests/Test3D.h"
#include "tests/TestChunk.h"

const int WINDOW_WIDTH = 2000;
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

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE); // request debug context

    //GLFWwindow* window = glfwCreateWindow(1000, 1414, "Hello World", nullptr, nullptr);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello World", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // vsync / set fps to monitor refresh rate


    if (glewInit() != GLEW_OK) {
        std::cout << "ERROR: GLEW NOT OK" << std::endl;
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

    std::cout << glGetString(GL_VERSION) << std::endl;


	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blending alfha
	glEnable(GL_BLEND);

	Renderer renderer;


    // imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");



	
	

    Test::Test* currentTest = nullptr;
	Test::TestMenu* testMenu = new Test::TestMenu(currentTest);
	currentTest = testMenu;

	testMenu->RegisterTest<Test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<Test::TestTexture>("Texture");
    testMenu->RegisterTest<Test::Test3D>("3D Cube");
	testMenu->RegisterTest<Test::TestChunk>("Voxel Chunk");


	
    while (!glfwWindowShouldClose(window)) // Update
    {
        renderer.Clear();



        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (currentTest){
            
			currentTest->OnUpdate(0.0f);
			currentTest->OnRender();
			ImGui::Begin("Test");
			if (currentTest != testMenu && ImGui::Button("<-"))
            {
                delete currentTest;
                currentTest = testMenu;
            }
			currentTest->OnImGuiRender();
            ImGui::End();

        }



        ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	delete currentTest;
    if (currentTest)
        delete testMenu;
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}