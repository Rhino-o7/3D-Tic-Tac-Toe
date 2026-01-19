#pragma once

#include <cstddef>
#include <thread>
#include <atomic>

#include "Test.h"
#include "voxel/ChunkBuilder.h"
#include "voxel/ChunkRenderer.h"
#include "voxel/Voxel.h"
#include "../VoxelClient.h"

namespace Test {

	class TestChunk : public Test {
	public:
		TestChunk();
		~TestChunk() override;

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImGuiRender() override;

	private:
		void UpdateChunksFromBoard();
		void RebuildAndUploadChunk(std::size_t index);
		void FillChunk(Chunk& chunk, VoxelType type);

		ChunkBuilder m_Builder;
		ChunkRenderer m_Renderer;
		std::size_t m_SelectedChunkIndex;
		VoxelClient m_VoxelClient;
		std::thread m_GameThread;
		int m_MoveIndexes[3] = { 0, 0, 0 };
		std::atomic<bool> m_NeedsUpdate{ false };
	};

}
