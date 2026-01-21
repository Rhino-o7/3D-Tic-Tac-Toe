#include "ChunkRenderer.h"

#include <algorithm>

#include "rendering/Renderer.h"
#include "rendering/VertexBufferLayout.h"
#include "imgui.h"

#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include <limits>

namespace
{
	constexpr float kNearClip = 0.1f;
	constexpr float kFarClip = 500.0f;
}

ChunkRenderer::ChunkRenderer(float aspectRatio)
	: m_Position(0.0f),
	m_Rotation(0.0f),
	m_Scale(1.0f),
	m_Camera(45.0f, aspectRatio, kNearClip, kFarClip),
	m_Fov(45.0f),
	m_AspectRatio(aspectRatio),
	m_SceneCenter(0.0f),
	m_CameraTarget(0.0f),
	m_CameraYaw(45.0f),
	m_CameraPitch(30.0f),
	m_CameraDistance(30.0f),
	m_HasGeometry(false)
{
	m_ClearColor[0] = 0.05f;
	m_ClearColor[1] = 0.05f;
	m_ClearColor[2] = 0.10f;
	m_ClearColor[3] = 1.0f;
}

ChunkRenderer::~ChunkRenderer()
{
	Unbind();
	m_RenderChunks.clear();
	m_HasGeometry = false;
}

void ChunkRenderer::EnsurePipeline()
{
	if (!m_Shader)
	{
		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
		m_Shader->Bind();
		m_Shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (!m_TextureAtlas.IsInitialized())
	{
		// Initialize with 2x2 grid for 32x32 texture atlas (16x16 per tile)
		m_TextureAtlas.Initialize("res/textures/voxel_atlas.png", 2, 2);
		
		// Map each voxel type to a tile position in the atlas
		// Format: MapVoxelTexture(VoxelType, tileX, tileY)
		m_TextureAtlas.MapVoxelTexture(VoxelType::Solid, 0, 0); // Top-left
		m_TextureAtlas.MapVoxelTexture(VoxelType::X, 1, 0);     // Top-right
		m_TextureAtlas.MapVoxelTexture(VoxelType::O, 0, 1);     // Bottom-left
		m_TextureAtlas.MapVoxelTexture(VoxelType::Hover, 1, 1); // Bottom-right
		
		m_Shader->SetUniform1i("u_Texture", 0);
	}
}

RenderChunkObj ChunkRenderer::BuildRenderChunk(const Mesh& mesh, const glm::vec3& offset)
{
	RenderChunkObj chunk;
	chunk.offset = offset;

	const std::size_t vertexCount = mesh.vertices.size() / 3;
	if (vertexCount == 0 || mesh.indices.empty())
	{
		return chunk;
	}

	std::vector<float> interleaved;
	interleaved.reserve(vertexCount * 5);

	const bool hasUvs = mesh.uvs.size() >= vertexCount * 2;
	const bool hasVoxelTypes = !mesh.voxelTypes.empty();

	for (std::size_t v = 0; v < vertexCount; ++v)
	{
		const std::size_t posIndex = v * 3;
		interleaved.push_back(mesh.vertices[posIndex + 0]);
		interleaved.push_back(mesh.vertices[posIndex + 1]);
		interleaved.push_back(mesh.vertices[posIndex + 2]);

		if (hasUvs && hasVoxelTypes)
		{
			const std::size_t faceIndex = v / 4;
			const VoxelType type = mesh.voxelTypes[faceIndex];
			
			const TextureCoords texCoords = m_TextureAtlas.GetTextureCoords(type);
			
			const std::size_t uvIndex = v * 2;
			const float u = mesh.uvs[uvIndex + 0];
			const float v_coord = mesh.uvs[uvIndex + 1];
			
			const float atlasU = texCoords.u_min + u * (texCoords.u_max - texCoords.u_min);
			const float atlasV = texCoords.v_min + v_coord * (texCoords.v_max - texCoords.v_min);
			
			interleaved.push_back(atlasU);
			interleaved.push_back(atlasV);
		}
		else
		{
			interleaved.push_back(0.0f);
			interleaved.push_back(0.0f);
		}
	}

	chunk.vao = std::make_unique<VertexArray>();
	chunk.vbo = std::make_unique<VertexBuffer>(
		interleaved.data(),
		static_cast<unsigned int>(interleaved.size() * sizeof(float)));

	VertexBufferLayout layout;
	layout.Push<float>(3); // Position
	layout.Push<float>(2); // UV
	chunk.vao->AddBuffer(*chunk.vbo, layout);

	chunk.ibo = std::make_unique<IndexBuffer>(
		mesh.indices.data(),
		static_cast<unsigned int>(mesh.indices.size()));

	chunk.visible = true;
	return chunk;
}

void ChunkRenderer::LoadChunks(const std::vector<Mesh>& meshes,
	const std::vector<glm::vec3>& offsets,
	const glm::ivec3& chunkDimensions)
{
	m_RenderChunks.clear();
	m_HasGeometry = false;

	if (meshes.empty() || meshes.size() != offsets.size())
	{
		return;
	}

	EnsurePipeline();

	glm::vec3 minBounds(
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max(),
		std::numeric_limits<float>::max());
	glm::vec3 maxBounds(
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest(),
		std::numeric_limits<float>::lowest());
	const glm::vec3 chunkSize = glm::vec3(chunkDimensions);

	for (std::size_t i = 0; i < meshes.size(); ++i)
	{
		const Mesh& mesh = meshes[i];
		RenderChunkObj renderChunk = BuildRenderChunk(mesh, offsets[i]);
		m_RenderChunks.emplace_back(std::move(renderChunk));

		minBounds = glm::min(minBounds, offsets[i]);
		maxBounds = glm::max(maxBounds, offsets[i] + chunkSize);
	}

	if (m_RenderChunks.empty())
	{
		m_HasGeometry = false;
		return;
	}

	RefreshGeometryState();

	m_SceneCenter = (minBounds + maxBounds) * 0.5f;
	m_CameraTarget = m_SceneCenter;
	m_Camera.SetTarget(m_CameraTarget);

	const float sceneSpan = glm::length(maxBounds - minBounds);
	const float chunkExtent = glm::max(glm::max(chunkSize.x, chunkSize.y), chunkSize.z);
	m_CameraDistance = glm::max(sceneSpan * 0.4f, chunkExtent * 2.0f);

	UpdateCameraFromOrbit();
}

void ChunkRenderer::UpdateChunkMesh(std::size_t index, const Mesh& mesh)
{
	if (index >= m_RenderChunks.size())
	{
		return;
	}

	const glm::vec3 offset = m_RenderChunks[index].offset;
	m_RenderChunks[index] = BuildRenderChunk(mesh, offset);

	RefreshGeometryState();
}

void ChunkRenderer::OnUpdate(float)
{
	if (!m_HasGeometry)
	{
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	if (!io.WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		const glm::vec2 delta(io.MouseDelta.x, io.MouseDelta.y);
		const float rotationSpeed = 0.2f;

		m_CameraYaw -= delta.x * rotationSpeed;
		m_CameraPitch += delta.y * rotationSpeed;
		m_CameraPitch = glm::clamp(m_CameraPitch, -85.0f, 85.0f);

		UpdateCameraFromOrbit();
	}

	if (!io.WantCaptureMouse && io.MouseWheel != 0.0f)
	{
		const float zoomSpeed = 0.8f;
		m_CameraDistance -= io.MouseWheel * zoomSpeed;
		m_CameraDistance = glm::clamp(m_CameraDistance, 2.0f, 500.0f);

		UpdateCameraFromOrbit();
	}
}

void ChunkRenderer::UpdateCameraFromOrbit()
{
	const float pitchRad = glm::radians(m_CameraPitch);
	const float yawRad = glm::radians(m_CameraYaw);

	glm::vec3 offset;
	offset.x = m_CameraDistance * cosf(pitchRad) * sinf(yawRad);
	offset.y = m_CameraDistance * sinf(pitchRad);
	offset.z = m_CameraDistance * cosf(pitchRad) * cosf(yawRad);

	const glm::vec3 position = m_CameraTarget + offset;

	m_Camera.SetPosition(position);
	m_Camera.SetTarget(m_CameraTarget);
}

void ChunkRenderer::OnRender()
{
	m_Camera.SetAspectRatio(m_AspectRatio);

	glEnable(GL_DEPTH_TEST);
	glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_HasGeometry)
	{
		glm::mat4 baseModel = glm::mat4(1.0f);
		baseModel = glm::translate(baseModel, m_Position);
		baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		baseModel = glm::scale(baseModel, m_Scale);

		EnsurePipeline();
		
		if (m_TextureAtlas.GetAtlasTexture())
		{
			m_TextureAtlas.GetAtlasTexture()->Bind();
		}
		
		m_Shader->Bind();

		const glm::mat4 projection = m_Camera.GetProjectionMatrix();
		const glm::mat4 view = m_Camera.GetViewMatrix();

		for (const RenderChunkObj& chunk : m_RenderChunks)
		{
			if (!chunk.visible || !chunk.vao || !chunk.ibo)
			{
				continue;
			}

			glm::mat4 model = glm::translate(baseModel, chunk.offset);
			glm::mat4 mvp = projection * view * model;

			m_Shader->SetUniformMat4f("u_MVP", mvp);
			Renderer::Draw(*chunk.vao, *chunk.ibo, *m_Shader);
		}

		Unbind();
	}

	glDisable(GL_DEPTH_TEST);
}

void ChunkRenderer::RefreshGeometryState()
{
	m_HasGeometry = std::any_of(
		m_RenderChunks.begin(),
		m_RenderChunks.end(),
		[](const RenderChunkObj& chunk)
		{
			return chunk.visible && chunk.vao && chunk.ibo;
		});
}

void ChunkRenderer::Unbind()
{
	if (m_Shader)
	{
		m_Shader->Unbind();
	}
	if (m_TextureAtlas.GetAtlasTexture())
	{
		m_TextureAtlas.GetAtlasTexture()->Unbind();
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void ChunkRenderer::OnImGuiRender()
{
	ImGui::SeparatorText("Chunk Renderer");
	ImGui::ColorEdit4("Clear Color", m_ClearColor);
	ImGui::SliderFloat3("Model Position", &m_Position.x, -100.0f, 100.0f);
	ImGui::SliderFloat3("Model Rotation (deg)", &m_Rotation.x, 0.0f, 360.0f);
	ImGui::SliderFloat3("Model Scale", &m_Scale.x, 0.1f, 5.0f);

	if (ImGui::SliderFloat("Field of View", &m_Fov, 25.0f, 90.0f))
	{
		m_Camera = Camera(m_Fov, m_AspectRatio, kNearClip, kFarClip);
		m_Camera.SetTarget(m_CameraTarget);
		UpdateCameraFromOrbit();
	}

	ImGui::Text("Camera Distance: %.2f", m_CameraDistance);
	ImGui::Text("Camera Yaw/Pitch: %.1f / %.1f", m_CameraYaw, m_CameraPitch);

	if (!m_HasGeometry)
	{
		ImGui::TextUnformatted("No chunk geometry loaded.");
	}
}

void ChunkRenderer::SetAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	m_Camera.SetAspectRatio(aspectRatio);
}
