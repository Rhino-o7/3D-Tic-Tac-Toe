#include "game.h"
#include <iostream>

Game::Game(Player clientPlayer) : client(clientPlayer) {
	ai = (clientPlayer == Player::X) ? Player::O : Player::X;
	currentTurn = Player::X; // X always start
}

void Game::printBoard() const {
	gameBoard.printBoardToConsole();
}

bool Game::takeTurn(Player player, int x, int y, int z) {
	if (player != currentTurn) {
		return false; 
	}

	if (!gameBoard.placeMark(player, x, y, z)) {
		return false; 
	}

	Player winner = gameBoard.checkWin();
	if (winner != Player::NONE) {
		// Handle win condition 

		std::cout << "Player " << ((winner == Player::X) ? 'X' : 'O') << " wins!\n";
		gameBoard.printBoardToConsole();
	}
	if (gameBoard.isFull()) {
		// Handle draw condition 
		gameBoard.printBoardToConsole();

		std::cout << "The game is a draw!\n";
	}

	currentTurn = (currentTurn == Player::X) ? Player::O : Player::X;

	//std::cout << " placed at (" << x << ", " << y << ", " << z << ")\n";
	

	return true;
}