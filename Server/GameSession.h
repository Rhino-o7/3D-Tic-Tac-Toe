#pragma once
#include "board.h"
#include "Game.h"
#include "WebSocketServer.h"
#include <memory>

class GameSession {
public:
	GameSession(ConnectionHandle client, WebSocketServer* server);
	
	void handleMessage(const NetworkMessage& msg);
	void startGame(Player clientPlayer);
	void processMove(const MoveData& move);
	void sendGameState();
	
	bool isActive() const { return active; }
	ConnectionHandle getClient() const { return clientHandle; }

private:
	void performAIMove();
	void handleGameOver();
	
	ConnectionHandle clientHandle;
	WebSocketServer* server;
	std::unique_ptr<Game> game;
	
	Player clientPlayer;
	Player aiPlayer;
	bool active;
};