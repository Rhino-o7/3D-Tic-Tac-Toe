#pragma once

#include <thread>
#include <atomic>
#include <optional>
#include <string>
#include <mutex>

#include "rendering/Renderer.h"
#include "voxel/ChunkBuilder.h"
#include "voxel/ChunkRenderer.h"
#include "voxel/Voxel.h"
#include "VoxelClient.h"

class Application {
public:
	Application(int windowWidth, int windowHeight);
	~Application();

	void OnUpdate(float deltaTime);
	void OnRender();
	void OnImGuiRender();
	void OnResize(int width, int height);

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
		glm::ivec3 chunkCoords; 
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
	std::atomic<bool> m_NeedsUpdate{ false };
	bool m_LastRightMouseState = false;
	std::optional<std::size_t> m_HoveredChunkIndex;
	
	// Connection and game states
	std::atomic<GameState> m_GameState;
	mutable std::mutex m_StatusMutex;
	std::string m_ServerAddress;
	std::string m_ConnectionStatus;
	Player m_Winner;
	
	// Window dimensions
	int m_WindowWidth;
	int m_WindowHeight;
};