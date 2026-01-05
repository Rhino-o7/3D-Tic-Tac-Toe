#pragma once


#include "board.h"


class Game {
public:
	Game(Player clinetPlayer);
	bool takeTurn(Player p, int x, int y, int z);
	void printBoard() const;
	Board& getBoard() { return gameBoard; }
	const Board& getBoard() const { return gameBoard; }
	Player checkWin() const { return gameBoard.checkWin(); }
	Player getCurrentTurn() const { return currentTurn; }
	bool isGameOver() const { return gameBoard.checkWin() != Player::NONE || gameBoard.isFull(); } //Cahneg later to not compute win every time
private:
	Player client, ai;
	Player currentTurn;
	Board gameBoard;
	
	
	
};
