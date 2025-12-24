#include "Game.h"

Game::Game(Player clientPlayer) : client(clientPlayer) {
	ai = (client == Player::X) ? Player::O : Player::X;
}

void Game::start() {
	gameBoard = Board();
}

bool Game::TakeTurn(Player player, int x, int y, int z) {
	return gameBoard.placeMark(player, x, y, z);
}