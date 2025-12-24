//#include "pch.h"
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
	if (board[x][y][z] != Player::NONE) {
		return false;
	}
	board[x][y][z] = player;
	return true;
}

void Board::undoMove(int x, int y, int z) {
	if (x >= 0 && x < 3 && y >= 0 && y < 3 && z >= 0 && z < 3) {
		board[x][y][z] = Player::NONE;
	}
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

Player Board::getCellState(int x, int y, int z) const {
	if (x < 0 || x >= 3 || y < 0 || y >= 3 || z < 0 || z >= 3) {
		return Player::NONE;
	}
	return board[x][y][z];
}

void Board::getCopyOfBoard(Player dest[3][3][3]) const {
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				dest[x][y][z] = board[x][y][z];
			}
		}
	}
}

void Board::PrintBoard() const {
	for (int z = 0; z < 3; ++z) {
		std::cout << "Layer " << z << ":\n";
		for (int y = 0; y < 3; ++y) {
			for (int x = 0; x < 3; ++x) {
				char mark;
				switch (board[x][y][z]) {
					case Player::X: mark = 'X'; break;
					case Player::O: mark = 'O'; break;
					default: mark = '.'; break;
				}
				std::cout << mark << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}
}

Player Board::checkWin() const {
	// Check all 27 rows (3 directions across 9 planes each)
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			// Z-axis rows
			if (board[i][j][0] != Player::NONE && 
				board[i][j][0] == board[i][j][1] && 
				board[i][j][1] == board[i][j][2]) {
				return board[i][j][0];
			}
			// Y-axis rows
			if (board[i][0][j] != Player::NONE && 
				board[i][0][j] == board[i][1][j] && 
				board[i][1][j] == board[i][2][j]) {
				return board[i][0][j];
			}
			// X-axis rows
			if (board[0][i][j] != Player::NONE && 
				board[0][i][j] == board[1][i][j] && 
				board[1][i][j] == board[2][i][j]) {
				return board[0][i][j];
			}
		}
	}
	
	// Check 18 plane diagonals
	for (int i = 0; i < 3; ++i) {
		// XY plane diagonals (z = i)
		if (board[0][0][i] != Player::NONE && 
			board[0][0][i] == board[1][1][i] && 
			board[1][1][i] == board[2][2][i]) {
			return board[0][0][i];
		}
		if (board[0][2][i] != Player::NONE && 
			board[0][2][i] == board[1][1][i] && 
			board[1][1][i] == board[2][0][i]) {
			return board[0][2][i];
		}
		// XZ plane diagonals (y = i)
		if (board[0][i][0] != Player::NONE && 
			board[0][i][0] == board[1][i][1] && 
			board[1][i][1] == board[2][i][2]) {
			return board[0][i][0];
		}
		if (board[0][i][2] != Player::NONE && 
			board[0][i][2] == board[1][i][1] && 
			board[1][i][1] == board[2][i][0]) {
			return board[0][i][2];
		}
		// YZ plane diagonals (x = i)
		if (board[i][0][0] != Player::NONE && 
			board[i][0][0] == board[i][1][1] && 
			board[i][1][1] == board[i][2][2]) {
			return board[i][0][0];
		}
		if (board[i][0][2] != Player::NONE && 
			board[i][0][2] == board[i][1][1] && 
			board[i][1][1] == board[i][2][0]) {
			return board[i][0][2];
		}
	}
	
	// Check 4 main diagonals through the center
	if (board[0][0][0] != Player::NONE && 
		board[0][0][0] == board[1][1][1] && 
		board[1][1][1] == board[2][2][2]) {
		return board[0][0][0];
	}
	if (board[0][0][2] != Player::NONE && 
		board[0][0][2] == board[1][1][1] && 
		board[1][1][1] == board[2][2][0]) {
		return board[0][0][2];
	}
	if (board[0][2][0] != Player::NONE && 
		board[0][2][0] == board[1][1][1] && 
		board[1][1][1] == board[2][0][2]) {
		return board[0][2][0];
	}
	if (board[2][0][0] != Player::NONE && 
		board[2][0][0] == board[1][1][1] && 
		board[1][1][1] == board[0][2][2]) {
		return board[2][0][0];
	}
	
	return Player::NONE;
}