#include "board.h"
#include <iostream>

Board::Board() {
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				board[x][y][z] = Player::NONE;
			}
		}
	}
}

bool Board::placeMark(Player player, int x, int y, int z) {
	if (x < 0 || x >= 3 || y < 0 || y >= 3 || z < 0 || z >= 3) {
		return false;
	}
	if (player != Player::NONE && board[x][y][z] != Player::NONE) {
		return false;
	}
	board[x][y][z] = player;
	return true;
}

void Board::printBoardToConsole() const {
	for (int i = 0; i < 3; i++) {
		std::cout << "\tz=" << i << "\t";
	}
	std::cout << std::endl;

	for (int row = 2; row >= 0; row--) {
		for (int z = 0; z < 3; z++) {
			if (row == 0) {
				std::cout << "(y" << row << ")[ ";
			}
			else {
				std::cout << "    [ ";
			}
			
			for (int col = 0; col < 3; col++) {
				char mark;
				switch (board[col][row][z]) {
					case Player::X: mark = 'X'; break;
					case Player::O: mark = 'O'; break;
					default: mark = '.'; break;
				}
				std::cout << mark << " ";
			}
			std::cout << "]\t";
		}
		std::cout << "\n";
	}
	for (int i = 0; i < 3; i++) {
		std::cout << "    (x0)\t";
	}
	std::cout << std::endl;
}

Player Board::getCell(int x, int y, int z) const {
	return board[x][y][z];
}

bool Board::isFull() const {
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				if (board[x][y][z] == Player::NONE) {
					return false;
				}
			}
		}
	}
	return true;
}

Player Board::checkWin() const {
	// Check rows along each axis
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// X-axis (varying x, fixed y and z)
			Player p = board[0][i][j];
			if (p != Player::NONE && board[1][i][j] == p && board[2][i][j] == p) {
				return p;
			}
			
			// Y-axis (fixed x, varying y, fixed z)
			p = board[i][0][j];
			if (p != Player::NONE && board[i][1][j] == p && board[i][2][j] == p) {
				return p;
			}
			
			// Z-axis (fixed x and y, varying z)
			p = board[i][j][0];
			if (p != Player::NONE && board[i][j][1] == p && board[i][j][2] == p) {
				return p;
			}
		}
	}
	
	// Check plane diagonals
	for (int i = 0; i < 3; i++) {
		// XY plane at z=i
		Player p = board[0][0][i];
		if (p != Player::NONE && board[1][1][i] == p && board[2][2][i] == p) return p;
		
		p = board[2][0][i];
		if (p != Player::NONE && board[1][1][i] == p && board[0][2][i] == p) return p;
		
		// XZ plane at y=i
		p = board[0][i][0];
		if (p != Player::NONE && board[1][i][1] == p && board[2][i][2] == p) return p;
		
		p = board[2][i][0];
		if (p != Player::NONE && board[1][i][1] == p && board[0][i][2] == p) return p;
		
		// YZ plane at x=i
		p = board[i][0][0];
		if (p != Player::NONE && board[i][1][1] == p && board[i][2][2] == p) return p;
		
		p = board[i][2][0];
		if (p != Player::NONE && board[i][1][1] == p && board[i][0][2] == p) return p;
	}
	
	// Check 4 space diagonals
	Player p = board[0][0][0];
	if (p != Player::NONE && board[1][1][1] == p && board[2][2][2] == p) return p;
	
	p = board[2][2][0];
	if (p != Player::NONE && board[1][1][1] == p && board[0][0][2] == p) return p;
	
	p = board[0][2][0];
	if (p != Player::NONE && board[1][1][1] == p && board[2][0][2] == p) return p;
	
	p = board[2][0][0];
	if (p != Player::NONE && board[1][1][1] == p && board[0][2][2] == p) return p;
	
	return Player::NONE;
}


