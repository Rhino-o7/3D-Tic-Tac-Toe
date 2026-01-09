#pragma once
#include <mutex>
#include "board.h"
#include "WebsocketClient.h"

class ConsoleClient {
private:
	Player player;
	std::unique_ptr<WebSocketClient> client;
	void onConnect();
	void onDisconnect();
	void onMessage(const NetworkMessage& msg);
	bool connected;

	// State management
	std::mutex stateMutex;
	GameStateData lastState{};
	bool hasState{ false };
	bool waitingForTurn{ false };
	bool running{ false };

	static constexpr int timeoutTime = 5000;

	// Helpers
	void sendPlayerChoice(Player choice);
	void promptAndSendMove();
	void printBoard(const GameStateData& state) const;
	void handleGameState(const GameStateData& state);

public:
	ConsoleClient();
	bool connectToServer(const std::string& uri);
};