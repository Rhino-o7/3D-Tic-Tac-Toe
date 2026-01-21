#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <glm.hpp>

#include "ChunkMesh.h"
#include "TextureAtlas.h"
#include "rendering/VertexArray.h"
#include "rendering/VertexBuffer.h"
#include "rendering/IndexBuffer.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Camera.h"

struct RenderChunkObj
{
	glm::vec3 offset{ 0.0f };
	std::unique_ptr<VertexArray> vao;
	std::unique_ptr<VertexBuffer> vbo;
	std::unique_ptr<IndexBuffer> ibo;
	bool visible{ false };
};

class ChunkRenderer
{
public:
	explicit ChunkRenderer(float aspectRatio);
	~ChunkRenderer();

	void LoadChunks(const std::vector<Mesh>& meshes,
		const std::vector<glm::vec3>& offsets,
		const glm::ivec3& chunkDimensions);

	void UpdateChunkMesh(std::size_t index, const Mesh& mesh);

	void OnUpdate(float deltaTime);
	void OnRender();
	void OnImGuiRender();

	void SetAspectRatio(float aspectRatio);
	
	// Camera access for raycasting
	const Camera& GetCamera() const { return m_Camera; }
	
	// Texture atlas management
	TextureAtlas& GetTextureAtlas() { return m_TextureAtlas; }

private:
	void UpdateCameraFromOrbit();
	void EnsurePipeline();
	RenderChunkObj BuildRenderChunk(const Mesh& mesh, const glm::vec3& offset);
	void RefreshGeometryState();
	void Unbind();

	std::vector<RenderChunkObj> m_RenderChunks;
	std::unique_ptr<Shader> m_Shader;
	TextureAtlas m_TextureAtlas;

	float m_ClearColor[4];
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_Scale;

	Camera m_Camera;
	float m_Fov;
	float m_AspectRatio;
	glm::vec3 m_SceneCenter;
	glm::vec3 m_CameraTarget;
	float m_CameraYaw;
	float m_CameraPitch;
	float m_CameraDistance;

	bool m_HasGeometry;
};

