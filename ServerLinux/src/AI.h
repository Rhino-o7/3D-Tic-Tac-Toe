#pragma once

#include "board.h"
#include <tuple>

class AI {
private:
	Player aiPlayer;
	Board* gameBoard;
	int skillLevel;
	int minimax(Board board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth);
public:
	AI(Player playerType, Board* board, int skillLvl) : aiPlayer(playerType), gameBoard(board), skillLevel(skillLvl) {}
	std::tuple<int, int, int> getBestMove();
	Player getPlayerType() const { return aiPlayer; }
	
};