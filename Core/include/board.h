#pragma once

using namespace std;

enum class Player {
	NONE = 0,
	X = 1,
	O = 2
};

class Board {
private:
	Player board[3][3][3];

public:
	Board();
	bool placeMark(Player player, int x, int y, int z);
	void undoMove(int x, int y, int z);
	Player checkWin() const;
	void PrintBoard() const;
	bool isFull() const;
	Player getCellState(int x, int y, int z) const;
	void getCopyOfBoard(Player dest[3][3][3]) const;
};