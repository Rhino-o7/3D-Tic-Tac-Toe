#pragma once

#include <memory>
#include <unordered_map>
#include "Voxel.h"
#include "rendering/Texture.h"

struct TextureCoords
{
	float u_min, v_min;
	float u_max, v_max;

	TextureCoords(float uMin = 0.0f, float vMin = 0.0f, float uMax = 1.0f, float vMax = 1.0f)
		: u_min(uMin), v_min(vMin), u_max(uMax), v_max(vMax) {}
};

class TextureAtlas
{
public:
	TextureAtlas();

	void Initialize(const std::string& atlasPath, int tilesX, int tilesY);
	void MapVoxelTexture(VoxelType type, int tileX, int tileY);
	
	TextureCoords GetTextureCoords(VoxelType type) const;
	Texture* GetAtlasTexture() const { return m_AtlasTexture.get(); }
	bool IsInitialized() const { return m_AtlasTexture != nullptr; }

private:
	std::unique_ptr<Texture> m_AtlasTexture;
	std::unordered_map<VoxelType, TextureCoords> m_VoxelTextureMap;
	int m_TilesX;
	int m_TilesY;
	float m_TileWidth;
	float m_TileHeight;
};