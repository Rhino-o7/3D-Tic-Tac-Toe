#pragma once
#include <mutex>
#include <functional>
#include <vector>
#include "board.h"
#include "WebsocketClient.h"
#include <glm.hpp>

class VoxelClient {
private:
	Player player;
	std::unique_ptr<WebSocketClient> client;
	void onConnect();
	void onDisconnect();
	void onMessage(const NetworkMessage& msg);
	bool connected;
	bool waitingForPlayerChoice = false;
	std::vector<glm::vec3> updatedChunks;
	mutable std::mutex stateMutex; // <-- Add 'mutable' here
	GameStateData lastState{};
	bool hasState{ false };
	bool waitingForTurn{ false };
	bool running{ false };
	static constexpr int timeoutTime = 5000;
	
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
		std::lock_guard<std::mutex> lock(stateMutex);
		return lastState;
	}
	
	std::vector<glm::vec3> GetUpdatedChunks() { 
		std::lock_guard<std::mutex> lock(stateMutex);
		return updatedChunks; 
	}
	
	void ClearUpdatedChunks() { 
		std::lock_guard<std::mutex> lock(stateMutex);
		updatedChunks.clear(); 
	}
	
	bool IsConnected() const { return connected; }

	bool IsWaitingForTurn() const { 
		std::lock_guard<std::mutex> lock(stateMutex);
		return waitingForTurn; 
	}
	
	bool SendMove(int x, int y, int z);
	bool connectToServer(const std::string& uri);
};