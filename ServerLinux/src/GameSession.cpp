#include "GameSession.h"

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
}

void GameSession::handlePlayerChoice(Player choice) { // Sets the X and O of Player and AI
	if (choice == Player::NONE) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Invalid player choice"));
		return;
	}


	clientPlayer = choice;
	aiPlayer = (choice == Player::X) ? Player::O : Player::X;

	game = std::make_unique<Game>(clientPlayer);
	ai = std::make_unique<AI>(aiPlayer, &game->getBoard(), 10); // Set Skill to 5, make var later

	// Check who goes first
	if (game->getCurrentTurn() == aiPlayer) {
		makeAiTurn();
	} else {
		sendGameState();
	}

}

void GameSession::handleMove(const MoveData& move) {
	// Check for errors
	if (!game) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Game not initialized; choose a player first."));
		return;
	}
	if (move.player != clientPlayer || game->getCurrentTurn() != clientPlayer) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Error! Not client turn"));
		return;
	}

	// Check for move error and apply move
	const bool applied = game->takeTurn(move.player, move.x, move.y, move.z);
	if (!applied) {
		server.sendMessage(client, NetworkMessage::createErrorMessage("Invalid move"));
		return;
	}

	sendGameState();

	// Ai turn
	if (!game->isGameOver() && game->getCurrentTurn() == aiPlayer) {
		makeAiTurn();
	}
}

void GameSession::makeAiTurn() {
	if (!game || !ai) {
		return;
	}

	// apply ai best move
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