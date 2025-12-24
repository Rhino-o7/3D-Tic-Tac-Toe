#pragma once
#include "board.h"


class Game {
public:
	Game(Player clientPlayer);
	void start();
	bool TakeTurn(Player player, int x, int y, int z);
private:
	Player client;
	Player ai;
	Board gameBoard;
};