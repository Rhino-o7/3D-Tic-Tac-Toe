#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "board.h"

enum class MessageType : uint8_t {
	CONNECT = 0,
	DISCONNECT = 1,
	PLAYER_MOVE = 2,
	GAME_STATE = 3,
	PLAYER_CHOICE = 4,
	GAME_START = 5,
	GAME_OVER = 6,
	ERROR_MSG = 7  // Changed from ERROR to avoid Windows macro conflict
};

struct MoveData {
	Player player;
	int x;
	int y;
	int z;
};

struct GameStateData {
	Player board[3][3][3];
	Player currentTurn;
	Player winner;
	bool isGameOver;
};

class NetworkMessage {
public:
	NetworkMessage(MessageType type);
	NetworkMessage(MessageType type, const std::string& payload);
	
	MessageType getType() const { return type; }
	std::string getPayload() const { return payload; }
	
	// Serialization
	std::string serialize() const;
	static NetworkMessage deserialize(const std::string& data);
	
	// Helper methods for specific message types
	static NetworkMessage createMoveMessage(Player player, int x, int y, int z);
	static NetworkMessage createGameStateMessage(const GameStateData& state);
	static NetworkMessage createPlayerChoiceMessage(Player choice);
	static NetworkMessage createErrorMessage(const std::string& error);
	
	// Parse specific message types
	MoveData parseMove() const;
	GameStateData parseGameState() const;
	Player parsePlayerChoice() const;

private:
	MessageType type;
	std::string payload;
};