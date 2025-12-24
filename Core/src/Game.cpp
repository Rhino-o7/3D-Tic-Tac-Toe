//#include "pch.h"
#include "Game.h"
#include <algorithm>
#include <limits>

Game::Game(Player clientPlayer) 
	: client(clientPlayer), 
	  currentTurn(Player::X),
	  gameOver(false),
	  winner(Player::NONE) {
	ai = (client == Player::X) ? Player::O : Player::X;
	gameBoard = Board();
}

void Game::start() {
	gameBoard = Board();
	currentTurn = Player::X;
	gameOver = false;
	winner = Player::NONE;
}

bool Game::TakeTurn(Player player, int x, int y, int z) {
	if (gameOver || player != currentTurn) {
		return false;
	}
	
	bool success = gameBoard.placeMark(player, x, y, z);
	
	if (success) {
		// Check for winner
		winner = gameBoard.checkWin();
		if (winner != Player::NONE) {
			gameOver = true;
		} else if (isDraw()) {
			gameOver = true;
		} else {
			// Switch turns
			currentTurn = (currentTurn == Player::X) ? Player::O : Player::X;
		}
	}
	
	return success;
}

Player Game::getWinner() const {
	return winner;
}

bool Game::isGameOver() const {
	return gameOver;
}

bool Game::isDraw() const {
	if (winner != Player::NONE) {
		return false;
	}
	return gameBoard.isFull();
}

void Game::printBoard() const {
	gameBoard.PrintBoard();
}

Move Game::getBestMove() {
	std::vector<Move> availableMoves = getAvailableMoves(gameBoard);
	
	if (availableMoves.empty()) {
		return Move();
	}
	
	// If first move, play center for better strategy
	if (availableMoves.size() == 27) {
		return Move(1, 1, 1);
	}
	
	Move bestMove;
	int bestScore = (std::numeric_limits<int>::min)();
	
	for (Move& move : availableMoves) {
		Board tempBoard = gameBoard;
		tempBoard.placeMark(ai, move.x, move.y, move.z);
		
		int score = minimax(tempBoard, 0, false, 
							(std::numeric_limits<int>::min)(), 
							(std::numeric_limits<int>::max)());
		
		if (score > bestScore) {
			bestScore = score;
			bestMove = move;
			bestMove.score = score;
		}
	}
	
	return bestMove;
}

int Game::minimax(Board& board, int depth, bool isMaximizing, int alpha, int beta) {
	Player result = board.checkWin();
	
	// Terminal states
	if (result == ai) {
		return 10 - depth; // Prefer faster wins
	}
	if (result == client) {
		return depth - 10; // Prefer slower losses
	}
	if (board.isFull()) {
		return 0; // Draw
	}
	
	// Depth limit to prevent excessive computation
	if (depth >= 6) {
		return evaluateBoard(board);
	}
	
	std::vector<Move> moves = getAvailableMoves(board);
	
	if (isMaximizing) {
		int maxScore = (std::numeric_limits<int>::min)();
		
		for (const Move& move : moves) {
			board.placeMark(ai, move.x, move.y, move.z);
			int score = minimax(board, depth + 1, false, alpha, beta);
			board.undoMove(move.x, move.y, move.z);
			
			maxScore = (std::max)(maxScore, score);
			alpha = (std::max)(alpha, score);
			
			if (beta <= alpha) {
				break; // Alpha-beta pruning
			}
		}
		
		return maxScore;
	} else {
		int minScore = (std::numeric_limits<int>::max)();
		
		for (const Move& move : moves) {
			board.placeMark(client, move.x, move.y, move.z);
			int score = minimax(board, depth + 1, true, alpha, beta);
			board.undoMove(move.x, move.y, move.z);
			
			minScore = (std::min)(minScore, score);
			beta = (std::min)(beta, score);
			
			if (beta <= alpha) {
				break; // Alpha-beta pruning
			}
		}
		
		return minScore;
	}
}

std::vector<Move> Game::getAvailableMoves(const Board& board) const {
	std::vector<Move> moves;
	
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = 0; z < 3; z++) {
				if (board.getCellState(x, y, z) == Player::NONE) {
					moves.push_back(Move(x, y, z));
				}
			}
		}
	}
	
	return moves;
}

int Game::evaluateBoard(const Board& board) const {
	// Simple heuristic evaluation
	// Could be enhanced with more sophisticated position evaluation
	return 0;
}