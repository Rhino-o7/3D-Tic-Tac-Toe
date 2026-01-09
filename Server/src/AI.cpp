#include "AI.h"
#include "board.h"
#include <algorithm>
#include <tuple>
#include <limits>
#include <iostream>

constexpr int MAX_DEPTH = 2;
constexpr int WIN_SCORE = 100;

// use to iteratre over board instead of nested loops
static inline void toXYZ(int idx, int& x, int& y, int& z) {
    x = idx / 9;
    y = (idx / 3) % 3;
    z = idx % 3;
}

int AI::minimax(Board board, int depth, bool isMaximizing, int alpha, int beta, int maxDepth) {
    Player winner = board.checkWin();
    Player opponent = (aiPlayer == Player::X) ? Player::O : Player::X;

    // Return win/loss score
    if (winner != Player::NONE) {
        return (winner == aiPlayer) ? (WIN_SCORE - depth) : (depth - WIN_SCORE);
    }

    if (board.isFull() || depth >= maxDepth) {
        return 0;
    }

    // minmax + pruning
    const Player current = isMaximizing ? aiPlayer : opponent;
    int best = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (board.getCell(x, y, z) != Player::NONE) {
            continue;
        }

        board.placeMark(current, x, y, z);
        int score = minimax(board, depth + 1, !isMaximizing, alpha, beta, maxDepth);

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

std::tuple<int, int, int> AI::getBestMove() {
    Board tmpBoard(*gameBoard);
    Player opponent = (aiPlayer == Player::X) ? Player::O : Player::X;

    std::cout << std::endl;

    int bestScore = std::numeric_limits<int>::min();
    int bestX = -1, bestY = -1, bestZ = -1;

    // Check if it can win
    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (tmpBoard.getCell(x, y, z) == Player::NONE) {
            tmpBoard.placeMark(aiPlayer, x, y, z);
            if (tmpBoard.checkWin() == aiPlayer) {
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

    // Minimax
    for (int idx = 0; idx < 27; ++idx) {
        int x, y, z; toXYZ(idx, x, y, z);
        if (tmpBoard.getCell(x, y, z) == Player::NONE) {
            Board candidateBoard(tmpBoard);
            candidateBoard.placeMark(aiPlayer, x, y, z);

            int score = minimax(candidateBoard, 1, false,
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(),
                MAX_DEPTH);

            if (score > bestScore) {
                bestScore = score;
                bestX = x;
                bestY = y;
                bestZ = z;
            }
        }
    }

    return std::make_tuple(bestX, bestY, bestZ);
}

