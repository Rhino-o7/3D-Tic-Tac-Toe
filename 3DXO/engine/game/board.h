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
	bool placeMark(Player player, int x, int y, int z); // This is to be replaced by networking
	Player checkWin();
	void PrintBoard();

	
};