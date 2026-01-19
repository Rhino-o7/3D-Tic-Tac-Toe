#include "ChunkBuilder.h"

#include <utility>

ChunkBuilder::ChunkBuilder() = default;

void ChunkBuilder::BuildChunks()
{
	m_Chunks.clear();
	m_ChunkOffsets.clear();
	m_ChunkMeshes.clear();

	const int totalChunks = NUM_CHUNKS_X * NUM_CHUNKS_Y * NUM_CHUNKS_Z;
	m_Chunks.reserve(totalChunks);
	m_ChunkOffsets.reserve(totalChunks);

	for (int x = 0; x < NUM_CHUNKS_X; ++x)
	{
		for (int y = 0; y < NUM_CHUNKS_Y; ++y)
		{
			for (int z = 0; z < NUM_CHUNKS_Z; ++z)
			{
				Chunk chunk(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z);
				chunk.SetChunkGame(1, x, y, z);

				m_Chunks.emplace_back(std::move(chunk));
				m_ChunkOffsets.emplace_back(
					static_cast<float>(x * CHUNK_SIZE_X),
					static_cast<float>(y * CHUNK_SIZE_Y),
					static_cast<float>(z * CHUNK_SIZE_Z));
			}
		}
	}
}

void ChunkBuilder::BuildChunksMesh()
{
	m_ChunkMeshes.clear();
	m_ChunkMeshes.reserve(m_Chunks.size());

	for (const Chunk& chunk : m_Chunks)
	{
		ChunkMesh chunkMesh;
		m_ChunkMeshes.emplace_back(chunkMesh.GenerateMesh(chunk));
	}
}

const std::vector<Mesh>& ChunkBuilder::GetChunkMeshes() const
{
	return m_ChunkMeshes;
}

const std::vector<glm::vec3>& ChunkBuilder::GetChunkOffsets() const
{
	return m_ChunkOffsets;
}

glm::ivec3 ChunkBuilder::GetChunkSize() const
{
	return { CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z };
}

glm::ivec3 ChunkBuilder::GetChunkGridSize() const
{
	return { NUM_CHUNKS_X, NUM_CHUNKS_Y, NUM_CHUNKS_Z };
}

std::size_t ChunkBuilder::GetChunkCount() const
{
	return m_Chunks.size();
}

Chunk* ChunkBuilder::GetChunk(std::size_t index)
{
	if (index >= m_Chunks.size())
	{
		return nullptr;
	}

	return &m_Chunks[index];
}

const Chunk* ChunkBuilder::GetChunk(std::size_t index) const
{
	if (index >= m_Chunks.size())
	{
		return nullptr;
	}

	return &m_Chunks[index];
}

const Mesh* ChunkBuilder::GetChunkMesh(std::size_t index) const
{
	if (index >= m_ChunkMeshes.size())
	{
		return nullptr;
	}

	return &m_ChunkMeshes[index];
}

const glm::vec3* ChunkBuilder::GetChunkOffset(std::size_t index) const
{
	if (index >= m_ChunkOffsets.size())
	{
		return nullptr;
	}

	return &m_ChunkOffsets[index];
}

bool ChunkBuilder::RebuildChunkMesh(std::size_t index)
{
	if (index >= m_Chunks.size())
	{
		return false;
	}

	if (m_ChunkMeshes.size() < m_Chunks.size())
	{
		m_ChunkMeshes.resize(m_Chunks.size());
	}

	ChunkMesh chunkMesh;
	m_ChunkMeshes[index] = chunkMesh.GenerateMesh(m_Chunks[index]);
	return true;
}
