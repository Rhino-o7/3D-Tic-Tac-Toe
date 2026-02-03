#pragma once

#include <memory>
#include <mutex>
#include "board.h"
#include "game.h"
#include "ai.h"
#include "WebSocketServer.h"

class GameSession {
public:
	GameSession(WebSocketServer& server, ConnectionHandle clientHandle);

	void HandleMessage(const NetworkMessage& msg);
	void HandleDisconnect();

private:
	void HandlePlayerChoice(Player choice);
	void HandleMove(const MoveData& move);
	void MakeAiTurn();
	void SendGameState();

	WebSocketServer& m_Server;
	ConnectionHandle m_Client;
	std::mutex m_SessionMutex;

	std::unique_ptr<Game> m_Game;
	std::unique_ptr<AI> m_AI;

	Player m_ClientPlayer{ Player::NONE };
	Player m_AIPlayer{ Player::NONE };
	const int m_AIDifficulty{ 5 }; // Depth for minimax
};
