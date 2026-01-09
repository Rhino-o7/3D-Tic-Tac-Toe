#include "network_message.h"
#include "board.h"
#include <sstream>

//Serialization

std::string NetworkMessage::serialize() const {
	std::ostringstream oss;
	oss << static_cast<int>(msg_type) << "|" << payload.size() << "|" << payload;
	return oss.str();

	// "2|12|X,1,2,3,..." <- (MessageType=2, length=12, data="X,1,2,3...")
}

NetworkMessage NetworkMessage::deserialize(const std::string& data) {
	std::istringstream iss(data);
	int msgTypeInt;
	size_t payloadSize;
	char delimiter;
	
	// messageType|payloadSize|payload
	iss >> msgTypeInt >> delimiter >> payloadSize >> delimiter;

	if (iss.fail()) {
		throw std::runtime_error("Failed to parse message header");
	}
	
	std::string payload;
	payload.resize(payloadSize);
	iss.read(&payload[0], payloadSize);
	
	MessageType msgType = static_cast<MessageType>(msgTypeInt);
	return NetworkMessage(msgType, payload);
}

// Creation functions

NetworkMessage NetworkMessage::createMoveMessage(const MoveData& move) {
	std::ostringstream oss;
	oss << static_cast<int>(move.player) << "," << move.x << "," << move.y << "," << move.z;
	return NetworkMessage(MessageType::MOVE, oss.str());
	
}

NetworkMessage NetworkMessage::createGameStateMessage(const GameStateData& state) {
	std::ostringstream oss;

	// board
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				oss << static_cast<int>(state.board[x][y][z]);
				if (x != 2 || y != 2 || z != 2) oss << ",";
			}
		}
	}

	// Add game state info
	oss << "|" << static_cast<int>(state.current_turn);
	oss << "|" << (state.game_over ? 1 : 0);
	oss << "|" << static_cast<int>(state.winner);

	return NetworkMessage(MessageType::GAME_STATE, oss.str());
}

NetworkMessage NetworkMessage::createPlayerChoiceMessage(Player choice) {
	return NetworkMessage(MessageType::PLAYER_CHOICE, std::to_string(static_cast<int>(choice)));
}

NetworkMessage NetworkMessage::createErrorMessage(const std::string& error) {
	return NetworkMessage(MessageType::ERROR_MSG, error);
}



// Parsing functions

MoveData NetworkMessage::parseMoveData() const {
	if (msg_type != MessageType::MOVE) {
		throw std::runtime_error("Cannot parse MoveData from non-MOVE message");
	}

	std::istringstream iss(payload);
	MoveData move;
	int playerInt;
	char comma;

	// Parse: "0,1,2,3" -> player=0, x=1, y=2, z=3
	iss >> playerInt >> comma >> move.x >> comma >> move.y >> comma >> move.z;

	if (iss.fail()) {
		throw std::runtime_error("Failed to parse MoveData from payload");
	}

	move.player = static_cast<Player>(playerInt);
	return move;
}

GameStateData NetworkMessage::parseGameStateData() const {
	if (msg_type != MessageType::GAME_STATE) {
		throw std::runtime_error(" Trying to parseGameStateData() a not GAME_STATE message");
	}

	std::istringstream iss(payload);
	GameStateData state;
	char comma, pipe;

	// Parse board
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				int cellValue;
				iss >> cellValue;
				state.board[x][y][z] = static_cast<Player>(cellValue);

				// Read comma (except for last cell)
				if (x != 2 || y != 2 || z != 2) {
					iss >> comma;
				}
			}
		}
	}

	// Parse game state
	int currentTurnInt, gameOverInt, winnerInt;
	iss >> pipe >> currentTurnInt >> pipe >> gameOverInt >> pipe >> winnerInt;

	if (iss.fail()) {
		throw std::runtime_error("Failed to parse GameStateData from payload");
	}

	state.current_turn = static_cast<Player>(currentTurnInt);
	state.game_over = (gameOverInt == 1);
	state.winner = static_cast<Player>(winnerInt);

	return state;
}

Player NetworkMessage::parsePlayerChoice() const {
	if (msg_type != MessageType::PLAYER_CHOICE) {
		throw std::runtime_error("Wrong message type for parsePlayerChoice");
	}

	return static_cast<Player>(std::stoi(payload));
}