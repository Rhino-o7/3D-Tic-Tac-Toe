#pragma once
#include <string>
#include "board.h"

enum class MessageType {
	CONNECT,
	DISCONNECT,
	MOVE,
	GAME_STATE,
	PLAYER_CHOICE,
	ERROR_MSG
	
};

struct MoveData {
	Player player;
	int x, y, z;
};
struct GameStateData {
	Player board[3][3][3];
	Player current_turn;
	bool game_over;
	Player winner;
};


class NetworkMessage {
public:
	NetworkMessage(MessageType msg_type) : msg_type(msg_type), payload("") {}
	NetworkMessage(MessageType msg_type, const std::string &payload) : msg_type(msg_type), payload(payload) {};

	MessageType getMessageType() const { return msg_type; }
	std::string getPayload() const { return payload; }

	// Serialization
	std::string serialize() const;
	static NetworkMessage deserialize(const std::string& data);

	// Creation functions
	static NetworkMessage createMoveMessage(const MoveData& move);
	static NetworkMessage createGameStateMessage(const GameStateData& state);
	static NetworkMessage createPlayerChoiceMessage(Player choice);
	static NetworkMessage createErrorMessage(const std::string& error);

	// Parsing functions
	MoveData parseMoveData() const;
	GameStateData parseGameStateData() const;
	Player parsePlayerChoice() const;

private:
	MessageType msg_type;
	std::string payload;
};
