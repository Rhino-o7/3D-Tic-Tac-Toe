#include "TestTexture.h"
#include "imgui.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

#include <GL/glew.h>

namespace Test {
	TestTexture::TestTexture()
        : m_Proj(glm::ortho(0.0f, 2000.0f, 0.0f, 1000.0f, -1.0f, 1.0f)), m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))),
            m_TranslationA(200, 200, 0), m_TranslationB(400,200,0)
		
    {


        float points[] {
            -300.0f, -300.0f, 0.0f, 0.0f, // bot left
             300.0f, -300.0f, 1.0f, 0.0f, // top right
             300.0f,  300.0f, 1.0f, 1.0f, // bot right
            -300.0f,  300.0f, 0.0f, 1.0f  // top left
        };

        unsigned int indices[] {
            0, 1, 2,
            2, 3, 0
        };

		
		m_VAO = std::make_unique<VertexArray>();

        
		m_VBO = std::make_unique<VertexBuffer>(points, 4 * 4 * sizeof(float));
        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);

        m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices, 6);

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
        m_Shader->Bind();
        m_Shader->SetUniform4f("u_Color", 0.0f, 0.2f, 0.9f, 1.0f);
        m_Texture = std::make_unique<Texture>("res/textures/insta.png");
        m_Shader->SetUniform1i("u_Texture", 0);

       
	}
	TestTexture::~TestTexture()
	{
        if (m_Shader)
        {
            m_Shader->Unbind();
        }

        if (m_Texture)
        {
            m_Texture->Unbind();
        }
	}
	void TestTexture::OnUpdate(float deltaTime)
	{
		
	}
	void TestTexture::OnRender()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		Renderer renderer;
		m_Texture->Bind();

       
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationA);
            glm::mat4 mvp = m_Proj * m_View * model;
            m_Shader->Bind();
            m_Shader->SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
        }
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationB);
            glm::mat4 mvp = m_Proj * m_View * model;
            m_Shader->Bind();
            m_Shader->SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(*m_VAO, *m_IBO, *m_Shader);
        }


	}
	void TestTexture::OnImGuiRender()
	{
        ImGui::SliderFloat3("Translate A", &m_TranslationA.x, 0.0f, 2000);
        ImGui::SliderFloat3("Translate B", &m_TranslationB.x, 0.0f, 2000);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}
}