#pragma once

#include <cstddef>
#include <thread>
#include <atomic>
#include <optional>
#include <string>
#include <mutex>

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
		enum class GameState {
			NotConnected,
			Connecting,
			WaitingForPlayerChoice,
			Playing,
			GameOver
		};

		struct Ray {
			glm::vec3 origin;
			glm::vec3 direction;
		};

		struct RayHit {
			bool hit;
			float distance;
			glm::ivec3 chunkCoords; // Grid coordinates (x, y, z) in 0-2 range
			std::size_t chunkIndex;
		};

		void UpdateChunksFromBoard();
		void RebuildAndUploadChunk(std::size_t index);
		void FillChunk(Chunk& chunk, VoxelType type);
		void HandleMouseInput();
		void UpdateHoverState();
		void ConnectToServer();
		void DisconnectFromServer();
		void CheckGameOver();
		void CheckDisconnection();
		void SetGameState(GameState state);
		GameState GetGameState() const;
		void SetConnectionStatus(const std::string& status);
		std::string GetConnectionStatus() const;
		Ray ScreenToWorldRay(double mouseX, double mouseY, int screenWidth, int screenHeight);
		RayHit RaycastChunks(const Ray& ray);
		bool RayAABBIntersection(const Ray& ray, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tMin);

		ChunkBuilder m_Builder;
		ChunkRenderer m_Renderer;
		std::size_t m_SelectedChunkIndex;
		VoxelClient m_VoxelClient;
		std::thread m_GameThread;
		int m_MoveIndexes[3] = { 0, 0, 0 };
		std::atomic<bool> m_NeedsUpdate{ false };
		bool m_LastRightMouseState = false;
		std::optional<std::size_t> m_HoveredChunkIndex;
		
		// Connection and game state (thread-safe)
		std::atomic<GameState> m_GameState;
		mutable std::mutex m_StatusMutex;
		std::string m_ServerAddress;
		std::string m_ConnectionStatus;
		Player m_Winner;
	};

}
