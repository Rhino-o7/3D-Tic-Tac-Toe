#include "../engine/game/board.h"
#include <iostream>

struct TestCoords {
	int coords[3][3];
};

int totalTests = 0;
int passedTests = 0;

void testWinCondition(const char* testName, const TestCoords& tc) {
	totalTests++;
	Board board;
	board.placeMark(Player::X, tc.coords[0][0], tc.coords[0][1], tc.coords[0][2]);
	board.placeMark(Player::X, tc.coords[1][0], tc.coords[1][1], tc.coords[1][2]);
	board.placeMark(Player::X, tc.coords[2][0], tc.coords[2][1], tc.coords[2][2]);
	
	if (board.checkWin() == Player::X) {
		std::cout << "[PASS] " << testName << "\n";
		passedTests++;
	} else {
		std::cout << "[FAIL] " << testName << " - Expected X to win\n";
	}
}
 
void testNoWin() {
	totalTests++;
	Board board;
	board.placeMark(Player::X, 0, 0, 0);
	board.placeMark(Player::O, 1, 1, 1);
	board.placeMark(Player::X, 2, 2, 2);
	
	if (board.checkWin() == Player::NONE) {
		std::cout << "[PASS] No false positive for non-winning position\n";
		passedTests++;
	} else {
		std::cout << "[FAIL] False positive detected\n";
	}
}

void runAllTests() {
	std::cout << "=== Testing 3D Tic-Tac-Toe Win Conditions ===\n\n";
	
	totalTests = 0;
	passedTests = 0;
	
	// Z-axis rows (9 tests)
	std::cout << "Testing Z-axis rows (9):\n";
	testWinCondition("Z-row [0,0,*]", {{{0,0,0},{0,0,1},{0,0,2}}});
	testWinCondition("Z-row [0,1,*]", {{{0,1,0},{0,1,1},{0,1,2}}});
	testWinCondition("Z-row [0,2,*]", {{{0,2,0},{0,2,1},{0,2,2}}});
	testWinCondition("Z-row [1,0,*]", {{{1,0,0},{1,0,1},{1,0,2}}});
	testWinCondition("Z-row [1,1,*]", {{{1,1,0},{1,1,1},{1,1,2}}});
	testWinCondition("Z-row [1,2,*]", {{{1,2,0},{1,2,1},{1,2,2}}});
	testWinCondition("Z-row [2,0,*]", {{{2,0,0},{2,0,1},{2,0,2}}});
	testWinCondition("Z-row [2,1,*]", {{{2,1,0},{2,1,1},{2,1,2}}});
	testWinCondition("Z-row [2,2,*]", {{{2,2,0},{2,2,1},{2,2,2}}});
	
	// Y-axis rows (9 tests)
	std::cout << "\nTesting Y-axis rows (9):\n";
	testWinCondition("Y-row [0,*,0]", {{{0,0,0},{0,1,0},{0,2,0}}});
	testWinCondition("Y-row [0,*,1]", {{{0,0,1},{0,1,1},{0,2,1}}});
	testWinCondition("Y-row [0,*,2]", {{{0,0,2},{0,1,2},{0,2,2}}});
	testWinCondition("Y-row [1,*,0]", {{{1,0,0},{1,1,0},{1,2,0}}});
	testWinCondition("Y-row [1,*,1]", {{{1,0,1},{1,1,1},{1,2,1}}});
	testWinCondition("Y-row [1,*,2]", {{{1,0,2},{1,1,2},{1,2,2}}});
	testWinCondition("Y-row [2,*,0]", {{{2,0,0},{2,1,0},{2,2,0}}});
	testWinCondition("Y-row [2,*,1]", {{{2,0,1},{2,1,1},{2,2,1}}});
	testWinCondition("Y-row [2,*,2]", {{{2,0,2},{2,1,2},{2,2,2}}});
	
	// X-axis rows (9 tests)
	std::cout << "\nTesting X-axis rows (9):\n";
	testWinCondition("X-row [*,0,0]", {{{0,0,0},{1,0,0},{2,0,0}}});
	testWinCondition("X-row [*,1,0]", {{{0,1,0},{1,1,0},{2,1,0}}});
	testWinCondition("X-row [*,2,0]", {{{0,2,0},{1,2,0},{2,2,0}}});
	testWinCondition("X-row [*,0,1]", {{{0,0,1},{1,0,1},{2,0,1}}});
	testWinCondition("X-row [*,1,1]", {{{0,1,1},{1,1,1},{2,1,1}}});
	testWinCondition("X-row [*,2,1]", {{{0,2,1},{1,2,1},{2,2,1}}});
	testWinCondition("X-row [*,0,2]", {{{0,0,2},{1,0,2},{2,0,2}}});
	testWinCondition("X-row [*,1,2]", {{{0,1,2},{1,1,2},{2,1,2}}});
	testWinCondition("X-row [*,2,2]", {{{0,2,2},{1,2,2},{2,2,2}}});
	
	// XY plane diagonals (6 tests)
	std::cout << "\nTesting XY plane diagonals (6):\n";
	testWinCondition("XY-diag z=0 main", {{{0,0,0},{1,1,0},{2,2,0}}});
	testWinCondition("XY-diag z=0 anti", {{{0,2,0},{1,1,0},{2,0,0}}});
	testWinCondition("XY-diag z=1 main", {{{0,0,1},{1,1,1},{2,2,1}}});
	testWinCondition("XY-diag z=1 anti", {{{0,2,1},{1,1,1},{2,0,1}}});
	testWinCondition("XY-diag z=2 main", {{{0,0,2},{1,1,2},{2,2,2}}});
	testWinCondition("XY-diag z=2 anti", {{{0,2,2},{1,1,2},{2,0,2}}});
	
	// XZ plane diagonals (6 tests)
	std::cout << "\nTesting XZ plane diagonals (6):\n";
	testWinCondition("XZ-diag y=0 main", {{{0,0,0},{1,0,1},{2,0,2}}});
	testWinCondition("XZ-diag y=0 anti", {{{0,0,2},{1,0,1},{2,0,0}}});
	testWinCondition("XZ-diag y=1 main", {{{0,1,0},{1,1,1},{2,1,2}}});
	testWinCondition("XZ-diag y=1 anti", {{{0,1,2},{1,1,1},{2,1,0}}});
	testWinCondition("XZ-diag y=2 main", {{{0,2,0},{1,2,1},{2,2,2}}});
	testWinCondition("XZ-diag y=2 anti", {{{0,2,2},{1,2,1},{2,2,0}}});
	
	// YZ plane diagonals (6 tests)
	std::cout << "\nTesting YZ plane diagonals (6):\n";
	testWinCondition("YZ-diag x=0 main", {{{0,0,0},{0,1,1},{0,2,2}}});
	testWinCondition("YZ-diag x=0 anti", {{{0,0,2},{0,1,1},{0,2,0}}});
	testWinCondition("YZ-diag x=1 main", {{{1,0,0},{1,1,1},{1,2,2}}});
	testWinCondition("YZ-diag x=1 anti", {{{1,0,2},{1,1,1},{1,2,0}}});
	testWinCondition("YZ-diag x=2 main", {{{2,0,0},{2,1,1},{2,2,2}}});
	testWinCondition("YZ-diag x=2 anti", {{{2,0,2},{2,1,1},{2,2,0}}});
	
	// 3D main diagonals (4 tests)
	std::cout << "\nTesting 3D main diagonals (4):\n";
	testWinCondition("3D-diag [0,0,0]->[2,2,2]", {{{0,0,0},{1,1,1},{2,2,2}}});
	testWinCondition("3D-diag [0,0,2]->[2,2,0]", {{{0,0,2},{1,1,1},{2,2,0}}});
	testWinCondition("3D-diag [0,2,0]->[2,0,2]", {{{0,2,0},{1,1,1},{2,0,2}}});
	testWinCondition("3D-diag [2,0,0]->[0,2,2]", {{{2,0,0},{1,1,1},{0,2,2}}});
	
	// Test no false positives
	std::cout << "\nTesting edge cases:\n";
	testNoWin();
	
	std::cout << "\n=== Test Results ===\n";
	std::cout << "Passed: " << passedTests << " / " << totalTests << "\n";
	if (passedTests == totalTests) {
		std::cout << "All tests passed! ?\n";
	} else {
		std::cout << "Failed: " << (totalTests - passedTests) << " test(s)\n";
	}
}

int main() {
	runAllTests();
	
	return 0;
}