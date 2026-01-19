#pragma once

#include <memory>
#include <glm.hpp>

#include "Test.h"
#include "rendering/VertexArray.h"
#include "rendering/VertexBuffer.h"
#include "rendering/VertexBufferLayout.h"
#include "rendering/IndexBuffer.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"

namespace Test {
	
	class Test3D : public Test {
	private:
		float m_ClearColor[4];
		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		glm::vec3 m_Scale;
		glm::vec3 m_CameraPosition;
		float m_Fov;
		float m_AspectRatio;
		glm::mat4 m_Proj;
		glm::mat4 m_View;
		std::unique_ptr<VertexArray> m_VAO;
		std::unique_ptr<VertexBuffer> m_VBO;
		std::unique_ptr<IndexBuffer> m_IBO;
		std::unique_ptr<Shader> m_Shader;
		std::unique_ptr<Texture> m_Texture;
	public:
		Test3D();
		~Test3D();
		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;
	};
	
}
