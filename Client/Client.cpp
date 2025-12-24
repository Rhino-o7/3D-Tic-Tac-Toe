#include "WebSocketClient.h"
#include "NetworkMessage.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <limits>

class GameClient {
public:
	GameClient() : running(false), myTurn(false), gameStarted(false), connectionEstablished(false), myPlayer(Player::NONE) {
		std::cout << "GameClient constructor called" << std::endl;
	}
	
	~GameClient() {
		cleanup();
	}
	
	bool run() {
		try {
			std::cout << "Setting up callbacks..." << std::endl;
			
			client.setOnConnectCallback([this]() { onConnect(); });
			client.setOnDisconnectCallback([this]() { onDisconnect(); });
			client.setOnMessageCallback([this](const NetworkMessage& msg) { onMessage(msg); });
			
			std::cout << "=== 3D Tic-Tac-Toe Client ===" << std::endl;
			std::cout << "Connecting to server..." << std::endl;
			
			client.connect("ws://localhost:9002");
			
			// Set running BEFORE starting threads
			running = true;
			
			// Wait for connection to establish (with timeout)
			std::cout << "Waiting for connection..." << std::endl;
			int timeout = 50; // 5 seconds
			while (!connectionEstablished && timeout > 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				timeout--;
			}
			
			if (!connectionEstablished) {
				std::cerr << "Failed to connect to server (timeout)" << std::endl;
				running = false;
				return false;
			}
			
			std::cout << "Connection established, starting game..." << std::endl;
			
			// Main game loop (runs in main thread)
			std::cout << "Starting game loop..." << std::endl;
			gameLoop();
			
			std::cout << "Game loop ended, cleaning up..." << std::endl;
			
			cleanup();
			
			std::cout << "Client shutting down cleanly" << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "Exception in run(): " << e.what() << std::endl;
			cleanup();
			return false;
		}
	}
	
	void cleanup() {
		running = false;
		if (connectionEstablished) {
			client.disconnect();
		}
		// Reset state for potential restart
		gameStarted = false;
		myTurn = false;
		connectionEstablished = false;
	}

private:
	void onConnect() {
		std::cout << "\n=== Connected to server! ===" << std::endl;
		connectionEstablished = true;
	}
	
	void onDisconnect() {
		std::cout << "\n=== Disconnected from server ===" << std::endl;
		running = false;
		connectionEstablished = false;
	}
	
	void onMessage(const NetworkMessage& msg) {
		try {
			std::cout << "Received message type: " << static_cast<int>(msg.getType()) << std::endl;
			
			switch (msg.getType()) {
				case MessageType::CONNECT: {
					std::cout << msg.getPayload() << std::endl;
					choosePlayer();
					break;
				}
				
				case MessageType::GAME_START: {
					myPlayer = static_cast<Player>(std::stoi(msg.getPayload()));
					gameStarted = true;
					std::cout << "\n=== Game Started! ===" << std::endl;
					std::cout << "You are playing as: " << (myPlayer == Player::X ? "X" : "O") << std::endl;
					std::cout << "Waiting for game state..." << std::endl;
					break;
				}
				
				case MessageType::GAME_STATE: {
					GameStateData state = msg.parseGameState();
					displayBoard(state);
					
					if (state.isGameOver) {
						handleGameOver(state.winner);
					} else {
						myTurn = (state.currentTurn == myPlayer);
						if (myTurn) {
							std::cout << "\n>>> YOUR TURN <<<" << std::endl;
						} else {
							std::cout << "\n>>> Waiting for opponent... <<<" << std::endl;
						}
					}
					break;
				}
				
				case MessageType::ERROR_MSG: {
					std::cout << "\n[ERROR] " << msg.getPayload() << std::endl;
					// If it's a move-related error and the game is active, allow retry
					if (gameStarted && !running.load()) {
						// Game already ended, don't restore turn
					} else if (gameStarted) {
						// Game is still active, give the player another chance
						myTurn = true;
						std::cout << "Please try again." << std::endl;
					}
					break;
				}
				
				case MessageType::GAME_OVER: {
					Player winner = static_cast<Player>(std::stoi(msg.getPayload()));
					handleGameOver(winner);
					break;
				}
				
				default:
					std::cout << "Unhandled message type: " << static_cast<int>(msg.getType()) << std::endl;
					break;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "Error in onMessage: " << e.what() << std::endl;
		}
	}
	
	void choosePlayer() {
		std::cout << "\nChoose your player:" << std::endl;
		std::cout << "1. X (goes first)" << std::endl;
		std::cout << "2. O (goes second)" << std::endl;
		std::cout << "Enter choice (1 or 2): ";
		
		int choice;
		while (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
			std::cin.clear(); // Clear error flags
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
			std::cout << "Invalid input! Please enter 1 or 2: ";
		}
		
		// Clear the input buffer after reading the choice
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		Player selectedPlayer = (choice == 1) ? Player::X : Player::O;
		NetworkMessage msg = NetworkMessage::createPlayerChoiceMessage(selectedPlayer);
		client.sendMessage(msg);
		
		std::cout << "Player choice sent: " << (selectedPlayer == Player::X ? "X" : "O") << std::endl;
	}
	
	void displayBoard(const GameStateData& state) {
		std::cout << "\n=== Current Board ===" << std::endl;
		
		for (int z = 0; z < 3; z++) {
			std::cout << "\nLayer " << z << " (z=" << z << "):" << std::endl;
			std::cout << "    x: 0 1 2" << std::endl;
			
			for (int y = 0; y < 3; y++) {
				std::cout << "y=" << y << ":   ";
				for (int x = 0; x < 3; x++) {
					char symbol;
					switch (state.board[x][y][z]) {
						case Player::X: symbol = 'X'; break;
						case Player::O: symbol = 'O'; break;
						default: symbol = '.'; break;
					}
					std::cout << symbol << " ";
				}
				std::cout << std::endl;
			}
		}
		std::cout << "\n====================\n" << std::endl;
	}
	
	void gameLoop() {
		std::cout << "Entered game loop (running=" << running << ", connected=" << connectionEstablished << ")" << std::endl;
		
		while (running && connectionEstablished) {
			if (gameStarted && myTurn) {
				makeMove();
				// Don't set myTurn to false here - let server response control it
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		
		std::cout << "Exited game loop (running=" << running << ", connected=" << connectionEstablished << ")" << std::endl;
	}
	
	void makeMove() {
		int x, y, z;
		bool validInput = false;
		
		// Set myTurn to false immediately so we don't re-enter while processing
		myTurn = false;
		
		while (!validInput) {
			std::cout << "\nEnter your move (x y z) separated by spaces:" << std::endl;
			std::cout << "Example: 1 1 1 (center of cube)" << std::endl;
			std::cout << "Range: 0-2 for each coordinate" << std::endl;
			std::cout << "Move: ";
			
			// Clear any error flags and input buffer before reading
			std::cin.clear();
			
			if (!(std::cin >> x >> y >> z)) {
				// Input failed (non-numeric input)
				std::cin.clear();
				std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				std::cout << "Invalid input! Please enter three numbers." << std::endl;
				continue;
			}
			
			// Clear remaining input buffer
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			
			if (x < 0 || x > 2 || y < 0 || y > 2 || z < 0 || z > 2) {
				std::cout << "Invalid coordinates! Must be 0-2" << std::endl;
				continue;
			}
			
			validInput = true;
		}
		
		NetworkMessage msg = NetworkMessage::createMoveMessage(myPlayer, x, y, z);
		client.sendMessage(msg);
		
		std::cout << "Move sent: (" << x << ", " << y << ", " << z << ")" << std::endl;
		std::cout << "Waiting for server response..." << std::endl;
	}
	
	void handleGameOver(Player winner) {
		std::cout << "\n========== GAME OVER ==========" << std::endl;
		
		if (winner == Player::NONE) {
			std::cout << "It's a DRAW!" << std::endl;
		} else if (winner == myPlayer) {
			std::cout << "?? YOU WIN! ??" << std::endl;
		} else {
			std::cout << "You lost. Better luck next time!" << std::endl;
		}
		
		std::cout << "===============================" << std::endl;
		
		// Stop the game loop and trigger cleanup
		gameStarted = false;
		running = false;
		myTurn = false;
	}
	
	WebSocketClient client;
	Player myPlayer;
	std::atomic<bool> running;
	std::atomic<bool> myTurn;
	std::atomic<bool> gameStarted;
	std::atomic<bool> connectionEstablished;
};

int main() {
	std::cout << "========================================" << std::endl;
	std::cout << "    3D Tic-Tac-Toe Game Client" << std::endl;
	std::cout << "========================================\n" << std::endl;
	
	bool playAgain = true;
	
	while (playAgain) {
		try {
			GameClient gameClient;
			std::cout << "GameClient created successfully\n" << std::endl;
			
			bool success = gameClient.run();
			
			if (success) {
				std::cout << "\nGameClient.run() completed successfully" << std::endl;
			} else {
				std::cout << "\nGameClient.run() failed to connect" << std::endl;
			}
			
		}
		catch (const std::exception& e) {
			std::cerr << "\nClient error: " << e.what() << std::endl;
		}
		
		// Ask if user wants to play again
		std::cout << "\n========================================" << std::endl;
		std::cout << "Would you like to play again?" << std::endl;
		std::cout << "1. Yes - Start a new game" << std::endl;
		std::cout << "2. No - Exit" << std::endl;
		std::cout << "Enter choice (1 or 2): ";
		
		int choice;
		std::cin.clear();
		while (!(std::cin >> choice) || (choice != 1 && choice != 2)) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid input! Please enter 1 or 2: ";
		}
		
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		
		if (choice == 2) {
			playAgain = false;
		} else {
			std::cout << "\n\n========================================" << std::endl;
			std::cout << "    Starting New Game..." << std::endl;
			std::cout << "========================================\n" << std::endl;
			
			// Small delay to let previous connection fully close
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	
	std::cout << "\n========================================" << std::endl;
	std::cout << "Thank you for playing!" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "\nPress Enter to exit...";
	std::cin.get();
	
	return 0;
}