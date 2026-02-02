#pragma once


#include "board.h"


class Game {
public:
	Game(Player clinetPlayer);
	bool takeTurn(Player p, int x, int y, int z);
	void printBoard() const;
	Board& getBoard() { return m_GameBoard; }
	const Board& getBoard() const { return m_GameBoard; }
	Player checkWin() const { return m_GameBoard.checkWin(); }
	Player getCurrentTurn() const { return m_CurrentTurn; }
	bool isGameOver() const { return m_GameBoard.checkWin() != Player::NONE || m_GameBoard.isFull(); } //Cahneg later to not compute win every time
private:
	Player m_Client, m_AI;
	Player m_CurrentTurn;
	Board m_GameBoard;
	
	
	
};
