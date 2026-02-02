#include "game.h"
#include <iostream>

Game::Game(Player clientPlayer) : m_Client(clientPlayer) {
	m_AI = (clientPlayer == Player::X) ? Player::O : Player::X;
	m_CurrentTurn = Player::X; // X always start
}

void Game::printBoard() const {
	m_GameBoard.printBoardToConsole();
}

bool Game::takeTurn(Player player, int x, int y, int z) {
	if (player != m_CurrentTurn) {
		return false; 
	}

	if (!m_GameBoard.placeMark(player, x, y, z)) {
		return false; 
	}

	Player winner = m_GameBoard.checkWin();
	if (winner != Player::NONE) {
		// Handle win condition 

		std::cout << "Player " << ((winner == Player::X) ? 'X' : 'O') << " wins!\n";
		m_GameBoard.printBoardToConsole();
	}
	if (m_GameBoard.isFull()) {
		// Handle draw condition 
		m_GameBoard.printBoardToConsole();

		std::cout << "The game is a draw!\n";
	}

	m_CurrentTurn = (m_CurrentTurn == Player::X) ? Player::O : Player::X;

	//std::cout << " placed at (" << x << ", " << y << ", " << z << ")\n";
	

	return true;
}