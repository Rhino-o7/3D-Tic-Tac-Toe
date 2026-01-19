#include "TextureAtlas.h"

TextureAtlas::TextureAtlas()
	: m_TilesX(1), m_TilesY(1), m_TileWidth(1.0f), m_TileHeight(1.0f)
{
}

void TextureAtlas::Initialize(const std::string& atlasPath, int tilesX, int tilesY)
{
	m_AtlasTexture = std::make_unique<Texture>(atlasPath);
	m_TilesX = tilesX;
	m_TilesY = tilesY;
	m_TileWidth = 1.0f / static_cast<float>(tilesX);
	m_TileHeight = 1.0f / static_cast<float>(tilesY);

	// Default mapping: all voxels use the first tile (top-left)
	TextureCoords defaultCoords(0.0f, 1.0f - m_TileHeight, m_TileWidth, 1.0f);
	m_VoxelTextureMap[VoxelType::Air] = defaultCoords;
	m_VoxelTextureMap[VoxelType::Solid] = defaultCoords;
	m_VoxelTextureMap[VoxelType::X] = defaultCoords;
	m_VoxelTextureMap[VoxelType::O] = defaultCoords;
	m_VoxelTextureMap[VoxelType::Other] = defaultCoords;
}

void TextureAtlas::MapVoxelTexture(VoxelType type, int tileX, int tileY)
{
	if (tileX < 0 || tileX >= m_TilesX || tileY < 0 || tileY >= m_TilesY)
	{
		return;
	}

	const float u_min = static_cast<float>(tileX) * m_TileWidth;
	const float u_max = u_min + m_TileWidth;
	
	// Flip V coordinate: tileY=0 should be at the TOP of the texture (V=1.0)
	const float v_max = 1.0f - (static_cast<float>(tileY) * m_TileHeight);
	const float v_min = v_max - m_TileHeight;

	m_VoxelTextureMap[type] = TextureCoords(u_min, v_min, u_max, v_max);
}

TextureCoords TextureAtlas::GetTextureCoords(VoxelType type) const
{
	auto it = m_VoxelTextureMap.find(type);
	if (it != m_VoxelTextureMap.end())
	{
		return it->second;
	}
	// Return top-left tile as default
	return TextureCoords(0.0f, 1.0f - m_TileHeight, m_TileWidth, 1.0f);
}