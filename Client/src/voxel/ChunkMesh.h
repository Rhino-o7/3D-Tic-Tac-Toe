#pragma once

#include <vector>

#include "Chunk.h"
#include "Voxel.h"

struct Mesh {
	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<unsigned int> indices;
	std::vector<VoxelType> voxelTypes; 
};

class ChunkMesh {
public:
	ChunkMesh() {}
	Mesh GenerateMesh(const Chunk& chunk);
};

