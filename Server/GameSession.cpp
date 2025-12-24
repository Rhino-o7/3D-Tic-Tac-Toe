#include "GameSession.h"
#include <iostream>

GameSession::GameSession(ConnectionHandle client, WebSocketServer* server)
	: clientHandle(client), server(server), active(false),
	  clientPlayer(Player::NONE), aiPlayer(Player::NONE) {
}

void GameSession::handleMessage(const NetworkMessage& msg) {
	switch (msg.getType()) {
		case MessageType::PLAYER_CHOICE: {
			Player choice = msg.parsePlayerChoice();
			startGame(choice);
			break;
		}
		case MessageType::PLAYER_MOVE: {
			MoveData move = msg.parseMove();
			processMove(move);
			break;
		}
		default:
			std::cout << "Unhandled message type" << std::endl;
	}
}

void GameSession::startGame(Player clientPlayer) {
	this->clientPlayer = clientPlayer;
	this->aiPlayer = (clientPlayer == Player::X) ? Player::O : Player::X;
	
	game = std::make_unique<Game>(clientPlayer);
	game->start();
	active = true;
	
	std::cout << "Game started. Client: " << static_cast<int>(clientPlayer) 
	          << ", AI: " << static_cast<int>(aiPlayer) << std::endl;
	
	NetworkMessage startMsg(MessageType::GAME_START, std::to_string(static_cast<int>(clientPlayer)));
	server->sendMessage(clientHandle, startMsg);
	
	sendGameState();
	
	// If AI goes first
	if (aiPlayer == Player::X) {
		performAIMove();
	}
}

void GameSession::processMove(const MoveData& move) {
	if (!active || !game) {
		return;
	}
	
	if (move.player != clientPlayer) {
		NetworkMessage errorMsg = NetworkMessage::createErrorMessage("Not your turn!");
		server->sendMessage(clientHandle, errorMsg);
		return;
	}
	
	if (game->isGameOver()) {
		NetworkMessage errorMsg = NetworkMessage::createErrorMessage("Game is over!");
		server->sendMessage(clientHandle, errorMsg);
		return;
	}
	
	bool validMove = game->TakeTurn(move.player, move.x, move.y, move.z);
	
	if (!validMove) {
		NetworkMessage errorMsg = NetworkMessage::createErrorMessage("Invalid move!");
		server->sendMessage(clientHandle, errorMsg);
		return;
	}
	
	std::cout << "Client played: (" << move.x << ", " << move.y << ", " << move.z << ")" << std::endl;
	
	sendGameState();
	
	// Check for game over
	if (game->isGameOver()) {
		handleGameOver();
		return;
	}
	
	// AI's turn
	performAIMove();
}

void GameSession::performAIMove() {
	if (!game || game->isGameOver()) {
		return;
	}
	
	std::cout << "AI thinking..." << std::endl;
	
	Move aiMove = game->getBestMove();
	
	if (aiMove.x == -1) {
		std::cerr << "AI couldn't find a valid move!" << std::endl;
		return;
	}
	
	bool success = game->TakeTurn(aiPlayer, aiMove.x, aiMove.y, aiMove.z);
	
	if (success) {
		std::cout << "AI played: (" << aiMove.x << ", " << aiMove.y << ", " << aiMove.z << ")" << std::endl;
		sendGameState();
		
		if (game->isGameOver()) {
			handleGameOver();
		}
	}
}

void GameSession::sendGameState() {
	if (!game) return;
	
	GameStateData state;
	game->getBoard().getCopyOfBoard(state.board);
	state.currentTurn = game->getCurrentTurn();
	state.winner = game->getWinner();
	state.isGameOver = game->isGameOver();
	
	NetworkMessage stateMsg = NetworkMessage::createGameStateMessage(state);
	server->sendMessage(clientHandle, stateMsg);
}

void GameSession::handleGameOver() {
	Player winner = game->getWinner();
	
	if (winner == Player::NONE) {
		std::cout << "Game ended in a draw!" << std::endl;
	} else if (winner == clientPlayer) {
		std::cout << "Client won!" << std::endl;
	} else {
		std::cout << "AI won!" << std::endl;
	}
	
	NetworkMessage gameOverMsg(MessageType::GAME_OVER, std::to_string(static_cast<int>(winner)));
	server->sendMessage(clientHandle, gameOverMsg);
	
	active = false;
}