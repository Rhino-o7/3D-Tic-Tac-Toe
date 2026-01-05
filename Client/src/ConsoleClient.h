#pragma once
#include <atomic>
#include <mutex>
#include "board.h"
#include "websocketClient.h"

class ConsoleClient {
private:
	Player player;
	WebSocketClient client;
	void onConnect();
	void onDisconnect();
	void onMessage(const NetworkMessage& msg);
	bool connected;
	int timeoutTime{ 5000 }; 

	// State management
	std::mutex stateMutex;
	GameStateData lastState{};
	bool hasState{ false };
	bool waitingForTurn{ false };
	bool running{ false };

	// Helpers
	void sendPlayerChoice(Player choice);
	void promptAndSendMove();
	void printBoard(const GameStateData& state) const;
	void handleGameState(const GameStateData& state);

public:
	ConsoleClient();
	bool connectToServer(const std::string& uri);
};