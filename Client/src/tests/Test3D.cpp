#include "Test3D.h"
#include "imgui.h"
#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include "rendering/Renderer.h"

namespace Test {
	Test3D::Test3D() 
		: m_Position(0.0f, 0.0f, 0.0f),
		  m_Rotation(0.0f, 0.0f, 0.0f),
		  m_Scale(1.0f, 1.0f, 1.0f),
		  m_CameraPosition(0.0f, 0.0f, 3.5f),
		  m_Fov(45.0f),
		  m_AspectRatio(2000.0f / 1000.0f),
		  m_Proj(glm::perspective(glm::radians(m_Fov), m_AspectRatio, 0.1f, 100.0f)),
		  m_View(glm::lookAt(m_CameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)))
		  
	{
		m_ClearColor[0] = 0.05f;
		m_ClearColor[1] = 0.05f;
		m_ClearColor[2] = 0.10f;
		m_ClearColor[3] = 1.0f;

		const float vertices[] = { //x,y,x ,u,v
			// Front
			-0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
			// Back
			-0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			// Left
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f, 1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
			// Right
			 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
			// Top
			-0.5f,  0.5f, -0.5f, 0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f, 1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f, 1.0f, 1.0f,
			// Bottom
			-0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f, 1.0f, 0.0f
		};

		const unsigned int indices[] = {
			 0,  1,  2,  2,  3,  0,
			 4,  5,  6,  6,  7,  4,
			 8,  9, 10, 10, 11,  8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20
		};

		m_VAO = std::make_unique<VertexArray>();
		m_VBO = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
		VertexBufferLayout layout;
		layout.Push<float>(3); //xyz
		layout.Push<float>(2); //uv
		m_VAO->AddBuffer(*m_VBO, layout);
		m_IBO = std::make_unique<IndexBuffer>(indices, static_cast<unsigned int>(sizeof(indices) / sizeof(unsigned int)));

		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
		m_Shader->Bind();
		m_Shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

		m_Texture = std::make_unique<Texture>("res/textures/dirt.jpg");
		m_Shader->SetUniform1i("u_Texture", 0);
	}

	Test3D::~Test3D()
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

	void Test3D::OnUpdate(float)
	{
	}

	void Test3D::OnRender()
	{
		m_View = glm::lookAt(m_CameraPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glEnable(GL_DEPTH_TEST);
		glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_Texture->Bind();

		glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_Position);
        model = glm::rotate(model, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, m_Scale);

		glm::mat4 mvp = m_Proj * m_View * model;

		m_Shader->Bind();
		m_Shader->SetUniformMat4f("u_MVP", mvp);
		Renderer::Draw(*m_VAO, *m_IBO, *m_Shader);

		glDisable(GL_DEPTH_TEST);
	}

	void Test3D::OnImGuiRender()
	{
		ImGui::ColorEdit4("Clear Color", m_ClearColor);
		ImGui::SliderFloat3("Model Position", &m_Position.x, -2.0f, 2.0f);
		ImGui::SliderFloat3("Model Rotation (deg)", &m_Rotation.x, 0.0f, 360.0f);
		ImGui::SliderFloat3("Model Scale", &m_Scale.x, 0.1f, 3.0f);

		if (ImGui::SliderFloat("Field of View", &m_Fov, 25.0f, 90.0f))
		{
			m_Proj = glm::perspective(glm::radians(m_Fov), m_AspectRatio, 0.1f, 100.0f);
		}

		ImGui::SliderFloat2("Camera Pan", &m_CameraPosition.x, -2.0f, 2.0f);

		float cameraDistance = m_CameraPosition.z;
		if (ImGui::SliderFloat("Camera Distance", &cameraDistance, 2.0f, 10.0f))
		{
			m_CameraPosition.z = cameraDistance;
		}

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
			1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
	}
}