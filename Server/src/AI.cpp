#include "AI.h"
#include <algorithm>
#include <tuple>
#include <limits>
#include <iostream>
const int MAX_DEPTH = 2;
int AI::minimax(Board board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth) {
	
	Player winner = board.checkWin();
	
	if (winner == aiPlayer) {
		return 100 - depth;
	}
	Player opponent = (aiPlayer == Player::X) ? Player::O : Player::X;
	if (winner == opponent) {
		return depth - 100;
	}
	if (board.isFull()) {
		return 0;
	}

	if (depth >= maxDepth) {
		return 0;
	}

	if (isMaximizing) {
		int maxScore = std::numeric_limits<int>::min();
		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {
				for (int z = 0; z < 3; z++) {
					if (board.getCell(x, y, z) == Player::NONE) {
						board.placeMark(aiPlayer, x, y, z);
						int score = minimax(board, depth + 1, false, alpha, beta, maxDepth);
						board.placeMark(Player::NONE, x, y, z);
						maxScore = std::max(maxScore, score);
						alpha = std::max(alpha, score);
						if (beta <= alpha) {
							return maxScore;
						}
					}
				}
			}
		}
		return maxScore;
	} else {
		int minScore = std::numeric_limits<int>::max();
		for (int x = 0; x < 3; x++) {
			for (int y = 0; y < 3; y++) {
				for (int z = 0; z < 3; z++) {
					if (board.getCell(x, y, z) == Player::NONE) {
						board.placeMark(opponent, x, y, z);
						int score = minimax(board, depth + 1, true, alpha, beta, maxDepth);
						board.placeMark(Player::NONE, x, y, z);
						minScore = std::min(minScore, score);
						beta = std::min(beta, score);
						if (beta <= alpha) {
							return minScore;
						}
					}
				}
			}
		}
		return minScore;
	}
}

std::tuple<int, int, int> AI::getBestMove() {
	Board tmpBoard(*gameBoard);
	Player opponent = (aiPlayer == Player::X) ? Player::O : Player::X;
	
	std::cout << std::endl;
	
	int bestScore = std::numeric_limits<int>::min();
	int bestX = -1, bestY = -1, bestZ = -1;
	
	// FIRST PRIORITY: Check if AI can win immediately - take the win!
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				if (tmpBoard.getCell(x, y, z) == Player::NONE) {
					tmpBoard.placeMark(aiPlayer, x, y, z);
					if (tmpBoard.checkWin() == aiPlayer) {
						tmpBoard.placeMark(Player::NONE, x, y, z);
						return std::make_tuple(x, y, z);
					}
					tmpBoard.placeMark(Player::NONE, x, y, z);
				}
			}
		}
	}

	// 2) Block opponent win
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				if (tmpBoard.getCell(x, y, z) == Player::NONE) {
					tmpBoard.placeMark(opponent, x, y, z);
					if (tmpBoard.checkWin() == opponent) {
						tmpBoard.placeMark(Player::NONE, x, y, z);
						return std::make_tuple(x, y, z);
					}
					tmpBoard.placeMark(Player::NONE, x, y, z);
				}
			}
		}
	}

	// 3) Minimax
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				if (tmpBoard.getCell(x, y, z) == Player::NONE) {
					tmpBoard.placeMark(aiPlayer, x, y, z);
					int score = minimax(tmpBoard, 1, false, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), MAX_DEPTH);
					tmpBoard.placeMark(Player::NONE, x, y, z);

					if (score > bestScore) {
						bestScore = score;
						bestX = x;
						bestY = y;
						bestZ = z;
					}
				}
			}
		}
	}

	return std::make_tuple(bestX, bestY, bestZ);
}

