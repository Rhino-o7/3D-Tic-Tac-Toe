#pragma once


enum class Player
{
	NONE,
	X,
	O
};

class Board {
private:
	Player board[3][3][3];
public:
	Board();
	bool placeMark(Player player, int x, int y, int z);
	Player checkWin() const;
	bool isFull() const;
	void printBoardToConsole() const;
	Player getCell(int x, int y, int z) const;

	
};