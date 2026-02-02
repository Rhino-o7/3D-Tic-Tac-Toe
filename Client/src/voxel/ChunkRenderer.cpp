#include "ChunkRenderer.h"

#include <algorithm>
#include <limits>

#include "rendering/Renderer.h"
#include "rendering/VertexBufferLayout.h"
#include "imgui.h"

#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>


// Clipping that fixes close up rendering artifacts
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
	m_RenderChunks.clear();
}

void ChunkRenderer::Validate()
{
	if (!m_Shader)
	{
		m_Shader = std::make_unique<Shader>("res/shaders/Basic.shader");
		m_Shader->Bind();
		m_Shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);
	}

	// Get textuers from texture atlas
	if (!m_TextureAtlas.IsInitialized())
	{
		m_TextureAtlas.Initialize("res/textures/voxel_atlas.png", 2, 2); 
		m_TextureAtlas.MapVoxelTexture(VoxelType::Solid, 0, 0);
		m_TextureAtlas.MapVoxelTexture(VoxelType::X, 1, 0);
		m_TextureAtlas.MapVoxelTexture(VoxelType::O, 0, 1);
		m_TextureAtlas.MapVoxelTexture(VoxelType::Hover, 1, 1);
		m_Shader->SetUniform1i("u_Texture", 0);
	}
}

// Builds a RenderChunkObj from a Mesh and offset
RenderChunkObj ChunkRenderer::BuildRenderChunk(const Mesh& mesh, const glm::vec3& offset)
{
	RenderChunkObj chunk;
	chunk.offset = offset;

	const std::size_t vertexCount = mesh.vertices.size() / 3;
	if (vertexCount == 0 || mesh.indices.empty())
	{
		return chunk;
	}

	const bool hasUvs = mesh.uvs.size() >= vertexCount * 2;
	const bool hasVoxelTypes = !mesh.voxelTypes.empty();

	std::vector<float> interleaved;
	interleaved.reserve(vertexCount * 5);

	for (std::size_t v = 0; v < vertexCount; ++v)
	{
		const std::size_t posIndex = v * 3;
		interleaved.push_back(mesh.vertices[posIndex]);
		interleaved.push_back(mesh.vertices[posIndex + 1]);
		interleaved.push_back(mesh.vertices[posIndex + 2]);

		if (hasUvs && hasVoxelTypes)
		{
			const TextureCoords texCoords = m_TextureAtlas.GetTextureCoords(mesh.voxelTypes[v / 4]);
			const std::size_t uvIndex = v * 2;
			const float u = mesh.uvs[uvIndex];
			const float v_coord = mesh.uvs[uvIndex + 1];
			
			interleaved.push_back(texCoords.u_min + u * (texCoords.u_max - texCoords.u_min));
			interleaved.push_back(texCoords.v_min + v_coord * (texCoords.v_max - texCoords.v_min));
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
	layout.Push<float>(3);
	layout.Push<float>(2);
	chunk.vao->AddBuffer(*chunk.vbo, layout);

	chunk.ibo = std::make_unique<IndexBuffer>(
		mesh.indices.data(),
		static_cast<unsigned int>(mesh.indices.size()));

	chunk.visible = true;
	return chunk;
}

// Loads multiple chunks into the renderer
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

	Validate();

	glm::vec3 minBounds(std::numeric_limits<float>::max());
	glm::vec3 maxBounds(std::numeric_limits<float>::lowest());
	const glm::vec3 chunkSize(chunkDimensions);

	for (std::size_t i = 0; i < meshes.size(); ++i)
	{
		m_RenderChunks.emplace_back(BuildRenderChunk(meshes[i], offsets[i]));
		minBounds = glm::min(minBounds, offsets[i]);
		maxBounds = glm::max(maxBounds, offsets[i] + chunkSize);
	}

	if (m_RenderChunks.empty())
	{
		return;
	}

	RefreshGeometryState();

	m_SceneCenter = (minBounds + maxBounds) * 0.5f;
	m_CameraTarget = m_SceneCenter;
	m_Camera.SetTarget(m_CameraTarget);

	const float sceneSpan = glm::length(maxBounds - minBounds);
	const float chunkExtent = glm::max(glm::max(chunkSize.x, chunkSize.y), chunkSize.z);
	m_CameraDistance = glm::max(sceneSpan * 0.6f, chunkExtent * 2.0f);

	UpdateCameraFromOrbit();
}

void ChunkRenderer::UpdateChunkMesh(std::size_t index, const Mesh& mesh)
{
	if (index >= m_RenderChunks.size())
	{
		return;
	}

	m_RenderChunks[index] = BuildRenderChunk(mesh, m_RenderChunks[index].offset);
	RefreshGeometryState();
}

// Update loop
void ChunkRenderer::OnUpdate(float)
{
	if (!m_HasGeometry)
	{
		return;
	}

	const ImGuiIO& io = ImGui::GetIO();

	if (!io.WantCaptureMouse && ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		constexpr float rotationSpeed = 0.2f;
		m_CameraYaw -= io.MouseDelta.x * rotationSpeed;
		m_CameraPitch = glm::clamp(m_CameraPitch + io.MouseDelta.y * rotationSpeed, -85.0f, 85.0f);
		UpdateCameraFromOrbit();
	}

	if (!io.WantCaptureMouse && io.MouseWheel != 0.0f)
	{
		constexpr float zoomSpeed = 0.8f;
		m_CameraDistance = glm::clamp(m_CameraDistance - io.MouseWheel * zoomSpeed, 2.0f, 500.0f);
		UpdateCameraFromOrbit();
	}
}

// Updates the camera position from orbit parameters
void ChunkRenderer::UpdateCameraFromOrbit()
{
	const float pitchRad = glm::radians(m_CameraPitch);
	const float yawRad = glm::radians(m_CameraYaw);

	const glm::vec3 offset(
		m_CameraDistance * cosf(pitchRad) * sinf(yawRad),
		m_CameraDistance * sinf(pitchRad),
		m_CameraDistance * cosf(pitchRad) * cosf(yawRad)
	);

	m_Camera.SetPosition(m_CameraTarget + offset);
	m_Camera.SetTarget(m_CameraTarget);
}

void ChunkRenderer::OnRender()
{
	m_Camera.SetAspectRatio(m_AspectRatio);


	glEnable(GL_DEPTH_TEST);
	glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!m_HasGeometry)
	{
		glDisable(GL_DEPTH_TEST);
		return;
	}

	Validate();
	
	if (m_TextureAtlas.GetAtlasTexture())
	{
		m_TextureAtlas.GetAtlasTexture()->Bind();
	}
	
	m_Shader->Bind();

	glm::mat4 baseModel = glm::mat4(1.0f);
	baseModel = glm::translate(baseModel, m_Position);
	baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	baseModel = glm::rotate(baseModel, glm::radians(m_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	baseModel = glm::scale(baseModel, m_Scale);

	const glm::mat4 projection = m_Camera.GetProjectionMatrix();
	const glm::mat4 view = m_Camera.GetViewMatrix();

	for (const RenderChunkObj& chunk : m_RenderChunks)
	{
		if (!chunk.visible || !chunk.vao || !chunk.ibo)
		{
			continue;
		}

		const glm::mat4 mvp = projection * view * glm::translate(baseModel, chunk.offset);
		m_Shader->SetUniformMat4f("u_MVP", mvp);
		Renderer::Draw(*chunk.vao, *chunk.ibo, *m_Shader);
	}

	Unbind();
	glDisable(GL_DEPTH_TEST);
}

void ChunkRenderer::RefreshGeometryState()
{
	m_HasGeometry = std::any_of(m_RenderChunks.begin(), m_RenderChunks.end(),
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

// Used when changine screen size during runtime
void ChunkRenderer::SetAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;
	m_Camera.SetAspectRatio(aspectRatio);
}
