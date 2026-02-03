#include "AI.h"
#include "board.h"
#include <algorithm>
#include <tuple>
#include <limits>
#include <iostream>
#include <random> 

constexpr int WIN_SCORE = 100;

// use to iteratre over board instead of nested loops
static inline void toXYZ(int idx, int& x, int& y, int& z) {
    x = idx / 9;
    y = (idx / 3) % 3;
    z = idx % 3;
}

int AI::Minimax(Board board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth) {
    Player winner = board.checkWin();
    Player opponent = (m_AIPlayer == Player::X) ? Player::O : Player::X;

    // Return win/loss score
    if (winner != Player::NONE) {
        return (winner == m_AIPlayer) ? (WIN_SCORE - depth) : (depth - WIN_SCORE);
    }

    if (board.isFull() || depth >= maxDepth) {
        return 0;
    }

    // minmax + pruning
    const Player current = isMaximizing ? m_AIPlayer : opponent;
    int best = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (board.getCell(x, y, z) != Player::NONE) {
            continue;
        }

        board.placeMark(current, x, y, z);
        int score = Minimax(board, depth + 1, !isMaximizing, alpha, beta, maxDepth);

        best = isMaximizing ? std::max(best, score) : std::min(best, score);

        // update alpha/beta
        if (isMaximizing) {
            alpha = std::max(alpha, score);
        } else {
            beta = std::min(beta, score);
        }

        // prune
        if (beta <= alpha) {
            break;
        }
    }

    return best;
}

std::tuple<int, int, int> AI::GetBestMove() {
    Board tmpBoard(*m_GameBoard);
    Player opponent = (m_AIPlayer == Player::X) ? Player::O : Player::X;

    std::cout << std::endl;

    int bestScore = std::numeric_limits<int>::min();
    int bestX = -1, bestY = -1, bestZ = -1;

    // Check if it can win
    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (tmpBoard.getCell(x, y, z) == Player::NONE) {
            tmpBoard.placeMark(m_AIPlayer, x, y, z);
            if (tmpBoard.checkWin() == m_AIPlayer) {
                return std::make_tuple(x, y, z);
            }
            tmpBoard.placeMark(Player::NONE, x, y, z);
        }
    }

    // Check for opponent win
    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (tmpBoard.getCell(x, y, z) == Player::NONE) {
            tmpBoard.placeMark(opponent, x, y, z);
            if (tmpBoard.checkWin() == opponent) {
                return std::make_tuple(x, y, z);
            }
            tmpBoard.placeMark(Player::NONE, x, y, z);
        }
    }

    std::vector<std::tuple<int,int,int>> bestMoves;

    for (int idx = 0; idx < 27; ++idx) {  // No shuffle
        int x, y, z; toXYZ(idx, x, y, z);
        if (tmpBoard.getCell(x, y, z) == Player::NONE) {
            Board candidateBoard(tmpBoard);
            candidateBoard.placeMark(m_AIPlayer, x, y, z);

            int score = Minimax(candidateBoard, 1, false,
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(),
                m_SkillLevel);

            if (score > bestScore) {
                bestScore = score;
                bestMoves.clear();
                bestMoves.push_back(std::make_tuple(x, y, z));
            } else if (score == bestScore) {
                bestMoves.push_back(std::make_tuple(x, y, z));
            }
        }
    }

    // Randomly pick from best movesw
    if (!bestMoves.empty()) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
        return bestMoves[dis(g)];
    }

    return std::make_tuple(bestX, bestY, bestZ);
}

