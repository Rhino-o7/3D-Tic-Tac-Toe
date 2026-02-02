#include "ChunkMesh.h"
#include "Voxel.h"

#include <array>

// Face UV coordinates and indices for a quad
namespace
{
	constexpr std::array<float, 8> kFaceUVs{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	constexpr std::array<unsigned int, 6> kFaceIndices{ 0, 1, 2, 2, 3, 0 };
}

// Create a single mesh from the chunk's voxels instead of rendering eaach voxel individually
Mesh ChunkMesh::GenerateMesh(const Chunk& chunk)
{
	Mesh chunkMesh;

	const auto& voxels = chunk.GetVoxels();
	if (voxels.empty())
	{
		return chunkMesh;
	}

	const int sizeX = chunk.GetSizeX();
	const int sizeY = chunk.GetSizeY();
	const int sizeZ = chunk.GetSizeZ();

	const std::size_t voxelCount = voxels.size();
	const std::size_t maxFaceCount = voxelCount * 6u;

	chunkMesh.vertices.reserve(maxFaceCount * 12u);
	chunkMesh.uvs.reserve(maxFaceCount * 8u);
	chunkMesh.indices.reserve(maxFaceCount * 6u);
	chunkMesh.voxelTypes.reserve(maxFaceCount);

	const VoxelFace faceTemplates;

	// append a face to the mesh
	const auto appendFace = [&](const float* faceVertices, int x, int y, int z, VoxelType type)
	{
		const unsigned int baseVertex = static_cast<unsigned int>(chunkMesh.vertices.size() / 3);

		for (int vertex = 0; vertex < 4; ++vertex)
		{
			const int offset = vertex * 3;
			chunkMesh.vertices.push_back(faceVertices[offset] + static_cast<float>(x));
			chunkMesh.vertices.push_back(faceVertices[offset + 1] + static_cast<float>(y));
			chunkMesh.vertices.push_back(faceVertices[offset + 2] + static_cast<float>(z));

			const int uvOffset = vertex * 2;
			chunkMesh.uvs.push_back(kFaceUVs[uvOffset]);
			chunkMesh.uvs.push_back(kFaceUVs[uvOffset + 1]);
		}

		for (const unsigned int indexOffset : kFaceIndices)
		{
			chunkMesh.indices.push_back(baseVertex + indexOffset);
		}

		chunkMesh.voxelTypes.push_back(type);
	};

	for (int i = 0; i < chunk.GetChunkCount(); ++i)
	{
		if (voxels[i] == VoxelType::Air)
		{
			continue;
		}

		// Set voxel faces based on neighbors
		const VoxelType currentType = voxels[i];
		const int x = i % sizeX;
		const int y = (i / sizeX) % sizeY;
		const int z = i / (sizeX * sizeY);

		const VoxelNeighbors neighbors = chunk.GetNeighbors(i);

		if (neighbors.front == VoxelType::Air)
		{
			appendFace(faceTemplates.frontFace, x, y, z, currentType);
		}

		if (neighbors.back == VoxelType::Air)
		{
			appendFace(faceTemplates.backFace, x, y, z, currentType);
		}

		if (neighbors.left == VoxelType::Air)
		{
			appendFace(faceTemplates.leftFace, x, y, z, currentType);
		}

		if (neighbors.right == VoxelType::Air)
		{
			appendFace(faceTemplates.rightFace, x, y, z, currentType);
		}

		if (neighbors.top == VoxelType::Air)
		{
			appendFace(faceTemplates.topFace, x, y, z, currentType);
		}

		if (neighbors.bottom == VoxelType::Air)
		{
			appendFace(faceTemplates.bottomFace, x, y, z, currentType);
		}
	}

	return chunkMesh;
}
