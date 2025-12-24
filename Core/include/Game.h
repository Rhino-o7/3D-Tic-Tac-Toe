#pragma once
#include "board.h"
#include <vector>

struct Move {
	int x, y, z;
	int score;
	
	Move() : x(-1), y(-1), z(-1), score(0) {}
	Move(int x, int y, int z) : x(x), y(y), z(z), score(0) {}
};

class Game {
public:
	Game(Player clientPlayer);
	void start();
	bool TakeTurn(Player player, int x, int y, int z);
	Player getWinner() const;
	bool isGameOver() const;
	bool isDraw() const;
	void printBoard() const;
	
	// AI functionality
	Move getBestMove();
	const Board& getBoard() const { return gameBoard; }
	Player getCurrentTurn() const { return currentTurn; }
	Player getClientPlayer() const { return client; }
	Player getAIPlayer() const { return ai; }

private:
	// Minimax algorithm
	int minimax(Board& board, int depth, bool isMaximizing, int alpha, int beta);
	std::vector<Move> getAvailableMoves(const Board& board) const;
	int evaluateBoard(const Board& board) const;
	
	Player client;
	Player ai;
	Board gameBoard;
	Player currentTurn;
	bool gameOver;
	Player winner;
};