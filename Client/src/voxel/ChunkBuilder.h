#pragma once

#include <cstddef>
#include <vector>
#include <glm.hpp>
#include "ChunkMesh.h"

class ChunkBuilder
{
public:
	ChunkBuilder();

	void BuildChunks();
	void BuildChunksMesh();

	const std::vector<Mesh>& GetChunkMeshes() const;
	const std::vector<glm::vec3>& GetChunkOffsets() const;
	glm::ivec3 GetChunkSize() const;
	glm::ivec3 GetChunkGridSize() const;
	std::size_t GetChunkCount() const;

	Chunk* GetChunk(std::size_t index);
	const Chunk* GetChunk(std::size_t index) const;
	const Mesh* GetChunkMesh(std::size_t index) const;
	const glm::vec3* GetChunkOffset(std::size_t index) const;
	bool RebuildChunkMesh(std::size_t index);

private:
	static constexpr int CHUNK_SIZE_X = 32;
	static constexpr int CHUNK_SIZE_Y = 32;
	static constexpr int CHUNK_SIZE_Z = 32;

	static constexpr int NUM_CHUNKS_X = 3;
	static constexpr int NUM_CHUNKS_Y = 3;
	static constexpr int NUM_CHUNKS_Z = 3;

	std::vector<Chunk> m_Chunks;
	std::vector<Mesh> m_ChunkMeshes;
	std::vector<glm::vec3> m_ChunkOffsets;
};

