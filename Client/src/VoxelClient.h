#pragma once
#include <mutex>
#include <functional>
#include <vector>
#include "board.h"
#include "WebsocketClient.h"
#include <glm.hpp>

class VoxelClient {
private:
	Player m_Player;
	std::unique_ptr<WebSocketClient> m_Client;
	void onConnect();
	void onDisconnect();
	void onMessage(const NetworkMessage& msg);
	bool m_Connected;
	bool m_WaitingForPlayerChoice = false;
	std::vector<glm::vec3> m_UpdatedChunks;
	mutable std::mutex m_StateMutex; 
	GameStateData m_LastState{};
	bool m_HasState{ false };
	bool m_WaitingForTurn{ false };
	bool m_Running{ false };
	static constexpr int m_TimeoutTime = 5000;
	
	// Callback for when game state changes
	std::function<void()> onStateChangeCallback;
	
	void handleGameState(const GameStateData& state);

public:
	VoxelClient();
	void StartGameLoop();
	void StopGameLoop();
	void SetPlayerChoice(Player choice);
	void SetOnStateChangeCallback(std::function<void()> callback) { onStateChangeCallback = callback; }

	GameStateData GetLastGameState() {
		std::lock_guard<std::mutex> lock(m_StateMutex);
		return m_LastState;
	}
	
	std::vector<glm::vec3> GetUpdatedChunks() { 
		std::lock_guard<std::mutex> lock(m_StateMutex);
		return m_UpdatedChunks; 
	}
	
	void ClearUpdatedChunks() { 
		std::lock_guard<std::mutex> lock(m_StateMutex);
		m_UpdatedChunks.clear(); 
	}
	
	bool IsConnected() const { return m_Connected; }

	bool IsWaitingForTurn() const { 
		std::lock_guard<std::mutex> lock(m_StateMutex);
		return m_WaitingForTurn; 
	}
	
	bool SendMove(int x, int y, int z);
	bool connectToServer(const std::string& uri);
};