#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>


#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui.h"


#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

const int WINDOW_WIDTH = 2000;
const int WINDOW_HEIGHT = 1000;

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*) {
    std::cout << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR ** " : "")
              << "type=" << std::hex << type
              << " severity=" << severity
              << " id=" << id
              << " msg=" << message << std::dec << std::endl;
    if (type == GL_DEBUG_TYPE_ERROR) {
		exit(1);
	}
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

    

    float points[] = {
       -50.0f, -50.0f, 0.0f, 0.0f, //bot left
        50.0f, -50.0f, 1.0f, 1.0f, //top right
        50.0f,  50.0f, 1.0f, 0.0f, //bot right
       -50.0f,  50.0f, 0.0f, 1.0f  //top left
	};

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
	};

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //blending alfha
	glEnable(GL_BLEND);

    VertexArray va;
	VertexBuffer vb(points, 4 * 4 * (sizeof(float)));
 
    VertexBufferLayout layout;
	layout.Push<float>(2);
    layout.Push<float>(2);
	va.AddBuffer(vb, layout);


	IndexBuffer ib(indices, 6);

	glm::mat4 proj = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_WIDTH, -1.0f, 1.0f);
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
	
    Shader shader("res/shaders/Basic.shader");
	shader.Bind();
	shader.SetUniform4f("u_Color", 0.0f, 0.2f, 0.9f, 1.0f);
    

	Texture texture("res/textures/ryo.png");
	texture.Bind();
	shader.SetUniform1i("u_Texture", 0);


	va.Unbind();
	vb.Unbind();
	ib.Unbind();
	shader.Unbind();

	Renderer renderer;


    // imgui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault();
    //io.Fonts->Build();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec3 translationA(200, 200, 0);
    glm::vec3 translationB(400, 200, 0);

    float r = 0.0f;
	float increment = 0.05f;

    /* Loop until the user closes the window    Update()   */ 
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        shader.Bind();
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), translationA);
            glm::mat4 mvp = proj * view * model;
            shader.SetUniformMat4f("u_MVP", mvp);

            renderer.Draw(va, ib, shader);  // Materials would pass (va, ib, material) material is (shader + uniforms)
        }
		// NOTE: instead of a for loop for many tiles and many Draw calls, do a sincgel draw call with one big VBO and IBO 
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), translationB);
            glm::mat4 mvp = proj * view * model;
            shader.SetUniformMat4f("u_MVP", mvp);

            renderer.Draw(va, ib, shader);
        }

        shader.SetUniform4f("u_Color", r, 0.2f, 0.9f, 1.0f);



        if (r > 1.0f) {
            increment = -0.05f;

        }
        else if (r < 0.0f) {
            increment = 0.05f;
        }

        r += increment;


        static float f = 0.0f;
        static int counter = 0;

        // imgui
        
        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
        ImGui::SliderFloat3("Translate A", &translationA.x, 0.0f, (float)WINDOW_WIDTH);
        ImGui::SliderFloat3("Translate B", &translationB.x, 0.0f, (float)WINDOW_WIDTH);// Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}