#include "Chunk.h"
#include <stdexcept>

Chunk::Chunk(int sizeX, int sizeY, int sizeZ) 
	: m_SizeX(sizeX), m_SizeY(sizeY), m_SizeZ(sizeZ),
		  m_ChunkX(0), m_ChunkY(0), m_ChunkZ(0),
	m_BorderWidth(0)
{
	m_TotalVoxels = sizeX * sizeY * sizeZ;
	m_Voxels.resize(m_TotalVoxels, VoxelType::Air);
}

void Chunk::SetIndex(int x, int y, int z, VoxelType type)
{
	if (x < 0 || x >= m_SizeX || y < 0 || y >= m_SizeY || z < 0 || z >= m_SizeZ)
		return;

	m_Voxels[x + m_SizeX * (y + m_SizeY * z)] = type;
}

void Chunk::SetChunkAll(VoxelType type)
{
	for (int x = 0; x < m_SizeX; ++x)
	{
		for (int y = 0; y < m_SizeZ; ++y)
		{
			for (int z = 0; z < m_SizeY; ++z)
			{
				SetIndex(x, y, z, type);
			}
		}
	}
}

void Chunk::SetChunkSolid()
{
	for (int x = 0; x < m_SizeX; ++x)
	{
		for (int y = 0; y < m_SizeZ; ++y)
		{
			for (int z = 0; z < m_SizeY; ++z)
			{
				SetIndex(x, y, z, VoxelType::Solid);
			}
		}
	}
}

void Chunk::SetChunkSomeSolid()
{
	for (int x = 0; x < m_SizeX; ++x)
	{
		for (int y = 0; y < m_SizeZ; ++y)
		{
			for (int z = 0; z < m_SizeY; ++z)
			{
				if (rand() % 100 < 70)
					SetIndex(x, y, z, VoxelType::Solid);
			}
		}
	}
}

void Chunk::SetChunkGame(int borderWidth, int chunkX, int chunkY, int chunkZ)
{
	m_BorderWidth = borderWidth;
	m_ChunkX = chunkX;
	m_ChunkY = chunkY;
	m_ChunkZ = chunkZ;
	SetChunkBorder(borderWidth);
	
}

void Chunk::SetChunkBorder(int borderWidth)
{
	const int thickness = borderWidth < 0 ? 0 : borderWidth;
	if (thickness == 0)
		return;

	constexpr int kMinChunkIndex = 0;
	constexpr int kMaxChunkIndex = 2;

	const bool hasNegX = m_ChunkX > kMinChunkIndex;
	const bool hasPosX = m_ChunkX < kMaxChunkIndex;
	const bool hasNegY = m_ChunkY > kMinChunkIndex;
	const bool hasPosY = m_ChunkY < kMaxChunkIndex;
	const bool hasNegZ = m_ChunkZ > kMinChunkIndex;
	const bool hasPosZ = m_ChunkZ < kMaxChunkIndex;

	for (int x = 0; x < m_SizeX; ++x)
	{
		for (int y = 0; y < m_SizeZ; ++y)
		{
			for (int z = 0; z < m_SizeY; ++z)
			{
				const bool nearNegX = hasNegX && x < thickness;
				const bool nearPosX = hasPosX && (m_SizeX - 1 - x) < thickness;
				const bool nearNegY = hasNegY && y < thickness;
				const bool nearPosY = hasPosY && (m_SizeZ - 1 - y) < thickness;
				const bool nearNegZ = hasNegZ && z < thickness;
				const bool nearPosZ = hasPosZ && (m_SizeY - 1 - z) < thickness;

				const bool touchesX = nearNegX || nearPosX;
				const bool touchesY = nearNegY || nearPosY;
				const bool touchesZ = nearNegZ || nearPosZ;

				const bool touchesEdge = (touchesX && touchesY) ||
					(touchesX && touchesZ) ||
					(touchesY && touchesZ);

				if (touchesEdge)
				{
					SetIndex(x, y, z, VoxelType::Solid);
				}
			}
		}
	}
}

void Chunk::SetChunkPlayer(Player player)
{

	switch (player)
	{
	case Player::NONE:
		SetChunkAll(VoxelType::Air);
		SetChunkBorder(m_BorderWidth);
		break;
	case Player::X:
		{
			const int minDimension = std::min({ m_SizeX, m_SizeY, m_SizeZ });
			const int thickness = std::max(1, minDimension / 16);
			const double normDenom = static_cast<double>(std::max(1, minDimension - 1));
			const double epsilon = static_cast<double>(thickness) / normDenom;

			struct AxisBounds
			{
				int start = 0;
				int end = 0;
				double invSpan = 0.0;
			};

			const auto makeBounds = [&](int size) -> AxisBounds
			{
				AxisBounds bounds{};
				const int maxMargin = std::max(0, (size - 2) / 2);
				const int axisMargin = std::min(std::max(1, thickness), maxMargin);

				bounds.start = (axisMargin > 0) ? axisMargin : 0;
				bounds.end = size - axisMargin;

				if (bounds.start >= bounds.end)
				{
					bounds.start = 0;
					bounds.end = size;
				}

				const int available = std::max(1, bounds.end - bounds.start - 1);
				bounds.invSpan = (available > 0) ? (1.0 / static_cast<double>(available)) : 0.0;
				return bounds;
			};

			const AxisBounds boundsX = makeBounds(m_SizeX);
			const AxisBounds boundsY = makeBounds(m_SizeY);
			const AxisBounds boundsZ = makeBounds(m_SizeZ);

			for (int x = boundsX.start; x < boundsX.end; ++x)
			{
				const double nx = (boundsX.invSpan > 0.0) ? (static_cast<double>(x - boundsX.start) * boundsX.invSpan) : 0.0;
				for (int y = boundsY.start; y < boundsY.end; ++y)
				{
					const double ny = (boundsY.invSpan > 0.0) ? (static_cast<double>(y - boundsY.start) * boundsY.invSpan) : 0.0;
					for (int z = boundsZ.start; z < boundsZ.end; ++z)
					{
						const double nz = (boundsZ.invSpan > 0.0) ? (static_cast<double>(z - boundsZ.start) * boundsZ.invSpan) : 0.0;

						const bool diag1 = (std::abs(nx - ny) <= epsilon) && (std::abs(ny - nz) <= epsilon);
						const bool diag2 = (std::abs(nx - ny) <= epsilon) && (std::abs((nx + nz) - 1.0) <= epsilon);
						const bool diag3 = (std::abs(nx - nz) <= epsilon) && (std::abs((nx + ny) - 1.0) <= epsilon);
						const bool diag4 = (std::abs(ny - nz) <= epsilon) && (std::abs((nx + ny) - 1.0) <= epsilon);

						if (diag1 || diag2 || diag3 || diag4)
						{
							SetIndex(x, y, z, VoxelType::X);
						}
					}
				}
			}
		}
		break;
	case Player::O:
		
	{
		int radius = m_SizeX / 4;

		const double centerX = (m_SizeX - 1) * 0.5;
		const double centerY = (m_SizeY - 1) * 0.5;
		const double centerZ = (m_SizeZ - 1) * 0.5;
		const double radiusSquared = radius * radius;

		for (int x = 0; x < m_SizeX; ++x)
		{
			for (int y = 0; y < m_SizeY; ++y)
			{
				for (int z = 0; z < m_SizeZ; ++z)
				{
					const double dx = static_cast<double>(x) - centerX;
					const double dy = static_cast<double>(y) - centerY;
					const double dz = static_cast<double>(z) - centerZ;
					const double distanceSquared = dx * dx + dy * dy + dz * dz;

					if (distanceSquared <= radiusSquared)
					{
						SetIndex(x, y, z, VoxelType::O);
					}
				}
			}
		}
	}
		break;

	default:
		break;
	}
}

VoxelType Chunk::GetIndex(int x, int y, int z) const
{
	if (x < 0 || x >= m_SizeX || y < 0 || y >= m_SizeY || z < 0 || z >= m_SizeZ)
		throw std::out_of_range("Chunk::GetIndex: Indices out of range");

	return m_Voxels[x + m_SizeX * (y + m_SizeY * z)];
}

VoxelNeighbors Chunk::GetNeighbors(int x, int y, int z) const
{
	VoxelNeighbors neighbors{};
	neighbors.back = (z > 0) ? GetIndex(x, y, z - 1) : VoxelType::Air;
	neighbors.front = (z < m_SizeZ - 1) ? GetIndex(x, y, z + 1) : VoxelType::Air;
	neighbors.left = (x > 0) ? GetIndex(x - 1, y, z) : VoxelType::Air;
	neighbors.right = (x < m_SizeX - 1) ? GetIndex(x + 1, y, z) : VoxelType::Air;
	neighbors.bottom = (y > 0) ? GetIndex(x, y - 1, z) : VoxelType::Air;
	neighbors.top = (y < m_SizeY - 1) ? GetIndex(x, y + 1, z) : VoxelType::Air;
	return neighbors;
}

VoxelNeighbors Chunk::GetNeighbors(int index) const
{

	int x = index % m_SizeX;
	int y = (index / m_SizeX) % m_SizeY;
	int z = index / (m_SizeX * m_SizeY);

	VoxelNeighbors neighbors{};
	neighbors.back = (z > 0) ? GetIndex(x, y, z - 1) : VoxelType::Air;
	neighbors.front = (z < m_SizeZ - 1) ? GetIndex(x, y, z + 1) : VoxelType::Air;
	neighbors.left = (x > 0) ? GetIndex(x - 1, y, z) : VoxelType::Air;
	neighbors.right = (x < m_SizeX - 1) ? GetIndex(x + 1, y, z) : VoxelType::Air;
	neighbors.bottom = (y > 0) ? GetIndex(x, y - 1, z) : VoxelType::Air;
	neighbors.top = (y < m_SizeY - 1) ? GetIndex(x, y + 1, z) : VoxelType::Air;
	return neighbors;
}





