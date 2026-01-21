#pragma once

#include <memory>
#include <mutex>
#include "board.h"
#include "game.h"
#include "AI.h"
#include "WebSocketServer.h"

class GameSession {
public:
	GameSession(WebSocketServer& server, ConnectionHandle clientHandle);

	void handleMessage(const NetworkMessage& msg);
	void handleDisconnect();

private:
	void handlePlayerChoice(Player choice);
	void handleMove(const MoveData& move);
	void makeAiTurn();
	void sendGameState();

	WebSocketServer& server;
	ConnectionHandle client;
	std::mutex sessionMutex;

	std::unique_ptr<Game> game;
	std::unique_ptr<AI> ai;

	Player clientPlayer{ Player::NONE };
	Player aiPlayer{ Player::NONE };
};
