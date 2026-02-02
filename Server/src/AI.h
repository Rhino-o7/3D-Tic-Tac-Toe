#pragma once

#include "board.h"
#include <tuple>

class AI {
private:
	Player m_AIPlayer;
	Board* m_GameBoard;
	int m_SkillLevel;
	int Minimax(Board board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth);
public:
	AI(Player playerType, Board* board, int skillLvl) : m_AIPlayer(playerType), m_GameBoard(board), m_SkillLevel(skillLvl) {}
	std::tuple<int, int, int> GetBestMove();
	Player GetPlayerType() const { return m_AIPlayer; }
	
};