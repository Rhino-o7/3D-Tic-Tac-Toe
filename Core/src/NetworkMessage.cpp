#include "NetworkMessage.h"
#include <sstream>
#include <stdexcept>

NetworkMessage::NetworkMessage(MessageType type) 
	: type(type), payload("") {}

NetworkMessage::NetworkMessage(MessageType type, const std::string& payload)
	: type(type), payload(payload) {}

std::string NetworkMessage::serialize() const {
	std::ostringstream oss;
	// Convert enum to char, not to ASCII digit
	oss << static_cast<char>(static_cast<uint8_t>(type)) << "|" << payload;
	return oss.str();
}

NetworkMessage NetworkMessage::deserialize(const std::string& data) {
	size_t delimPos = data.find('|');
	if (delimPos == std::string::npos || delimPos == 0) {
		throw std::runtime_error("Invalid message format");
	}
	
	// Read the type as a byte, not as ASCII digit
	uint8_t typeVal = static_cast<uint8_t>(data[0]);
	MessageType type = static_cast<MessageType>(typeVal);
	std::string payload = data.substr(delimPos + 1);
	
	return NetworkMessage(type, payload);
}

NetworkMessage NetworkMessage::createMoveMessage(Player player, int x, int y, int z) {
	std::ostringstream oss;
	oss << static_cast<int>(player) << "," << x << "," << y << "," << z;
	return NetworkMessage(MessageType::PLAYER_MOVE, oss.str());
}

NetworkMessage NetworkMessage::createGameStateMessage(const GameStateData& state) {
	std::ostringstream oss;
	
	// Serialize board state
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				oss << static_cast<int>(state.board[x][y][z]);
			}
		}
	}
	
	oss << "|" << static_cast<int>(state.currentTurn);
	oss << "|" << static_cast<int>(state.winner);
	oss << "|" << (state.isGameOver ? "1" : "0");
	
	return NetworkMessage(MessageType::GAME_STATE, oss.str());
}

NetworkMessage NetworkMessage::createPlayerChoiceMessage(Player choice) {
	return NetworkMessage(MessageType::PLAYER_CHOICE, std::to_string(static_cast<int>(choice)));
}

NetworkMessage NetworkMessage::createErrorMessage(const std::string& error) {
	return NetworkMessage(MessageType::ERROR_MSG, error);
}

MoveData NetworkMessage::parseMove() const {
	if (type != MessageType::PLAYER_MOVE) {
		throw std::runtime_error("Wrong message type for parseMove");
	}
	
	MoveData move;
	std::istringstream iss(payload);
	std::string token;
	
	std::getline(iss, token, ',');
	move.player = static_cast<Player>(std::stoi(token));
	
	std::getline(iss, token, ',');
	move.x = std::stoi(token);
	
	std::getline(iss, token, ',');
	move.y = std::stoi(token);
	
	std::getline(iss, token, ',');
	move.z = std::stoi(token);
	
	return move;
}

GameStateData NetworkMessage::parseGameState() const {
	if (type != MessageType::GAME_STATE) {
		throw std::runtime_error("Wrong message type for parseGameState");
	}
	
	GameStateData state;
	std::istringstream iss(payload);
	std::string boardData, turnData, winnerData, gameOverData;
	
	std::getline(iss, boardData, '|');
	std::getline(iss, turnData, '|');
	std::getline(iss, winnerData, '|');
	std::getline(iss, gameOverData, '|');
	
	// Deserialize board
	int idx = 0;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				state.board[x][y][z] = static_cast<Player>(boardData[idx++] - '0');
			}
		}
	}
	
	state.currentTurn = static_cast<Player>(std::stoi(turnData));
	state.winner = static_cast<Player>(std::stoi(winnerData));
	state.isGameOver = (gameOverData == "1");
	
	return state;
}

Player NetworkMessage::parsePlayerChoice() const {
	if (type != MessageType::PLAYER_CHOICE) {
		throw std::runtime_error("Wrong message type for parsePlayerChoice");
	}
	
	return static_cast<Player>(std::stoi(payload));
}