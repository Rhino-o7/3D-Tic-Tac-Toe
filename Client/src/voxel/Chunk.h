#pragma once
#include "Voxel.h"
#include <vector>
#include "board.h"



class Chunk
{
private:
	int m_SizeX, m_SizeY, m_SizeZ;
	int m_TotalVoxels;
	std::vector<VoxelType> m_Voxels;
	int m_ChunkX, m_ChunkY, m_ChunkZ;
	int m_BorderWidth;
public:
	Chunk(int sizeX, int sizeY, int sizeZ);
	~Chunk() {}

	std::vector<VoxelType>& GetVoxels() { return m_Voxels; }
	const std::vector<VoxelType>& GetVoxels() const { return m_Voxels; }
	int GetChunkCount() const { return m_TotalVoxels; }
	int GetSizeX() const { return m_SizeX; }
	int GetSizeY() const { return m_SizeY; }
	int GetSizeZ() const { return m_SizeZ; }

	void SetChunkAll(VoxelType type);
	void SetChunkHover();
	void ClearChunkHover();
	void SetChunkSolid();
	void SetChunkSomeSolid();
	void ClearButBorder();

	void SetChunkGame(int borderWidth, int chunkX, int chunkY, int chunkZ);
	void SetChunkBorder(int borderWidth);
	void SetChunkPlayer(Player player);

	VoxelType GetIndex(int x, int y, int z) const;
	VoxelNeighbors GetNeighbors(int x, int y, int z) const;
	VoxelNeighbors GetNeighbors(int index) const;
	void SetIndex(int x, int y, int z, VoxelType type);

};

