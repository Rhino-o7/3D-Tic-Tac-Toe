#include "GameSession.h"

#include <iostream>

GameSession::GameSession(WebSocketServer& server, ConnectionHandle clientHandle)
	: server(server), client(clientHandle) {
}

void GameSession::handleMessage(const NetworkMessage& msg) {
	std::lock_guard<std::mutex> lock(sessionMutex);

	switch (msg.getMessageType()) {
	case MessageType::PLAYER_CHOICE:
		handlePlayerChoice(msg.parsePlayerChoice());
		break;
	case MessageType::MOVE:
		handleMove(msg.parseMoveData());
		break;
	default:
		server.sendMessage(client, NetworkMessage::createErrorMessage("Unsupported message type"));
		break;
	}
}

void GameSession::handleDisconnect() {
	std::lock_guard<std::mutex> lock(sessionMutex);
	// Place any per-session cleanup you need here.
}

void GameSession::handlePlayerChoice(Player choice) {
	if (choice == Player::NONE) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Invalid player choice"));
		return;
	}

	clientPlayer = choice;
	aiPlayer = (choice == Player::X) ? Player::O : Player::X;

	game = std::make_unique<Game>(clientPlayer);
	ai = std::make_unique<AI>(aiPlayer, &game->getBoard(), 5);

	// X always starts; if AI is X, let it move immediately.
	if (game->getCurrentTurn() == aiPlayer) {
		makeAiTurn();
	} else {
		sendGameState();
	}
}

void GameSession::handleMove(const MoveData& move) {
	if (!game) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Game not initialized; choose a player first."));
		return;
	}

	if (move.player != clientPlayer) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("It is not your side."));
		return;
	}

	if (game->getCurrentTurn() != clientPlayer) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Not your turn."));
		return;
	}

	// Let the caller fill in real game logic; this just forwards the move.
	const bool applied = game->takeTurn(move.player, move.x, move.y, move.z);
	if (!applied) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Invalid move"));
		return;
	}

	sendGameState();

	// If the game continues and it's now the AI's turn, let AI play.
	if (!game->isGameOver() && game->getCurrentTurn() == aiPlayer) {
		makeAiTurn();
	}
}

void GameSession::makeAiTurn() {
	if (!game || !ai) {
		return;
	}

	auto [x, y, z] = ai->getBestMove();
	const bool applied = game->takeTurn(aiPlayer, x, y, z);
	if (!applied) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("AI failed to place move"));
		return;
	}

	sendGameState();
}

void GameSession::sendGameState() {
	if (!game) {
		return;
	}

	GameStateData state{};
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				state.board[x][y][z] = game->getBoard().getCell(x, y, z);
			}
		}
	}

	state.current_turn = game->getCurrentTurn();
	state.game_over = game->isGameOver();
	state.winner = game->checkWin();
	

	server.sendMessage(client, NetworkMessage::createGameStateMessage(state));
}