#include <iostream>
#include <limits>
#include <thread>
#include "ConsoleClient.h"

using namespace std;

ConsoleClient::ConsoleClient() : player(Player::X), connected(false) {
	client = std::make_unique<WebSocketClient>();
}

void ConsoleClient::onConnect() {
	std::cout << "Connected to server." << std::endl;
	connected = true;
}

void ConsoleClient::onDisconnect() {
	std::cout << "Disconnected from server." << std::endl;
	connected = false;
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		waitingForTurn = false;
		hasState = false;
	}
	running = false;
}

void ConsoleClient::onMessage(const NetworkMessage& msg) {
	switch (msg.getMessageType()) {
	case MessageType::CONNECT:
		std::cout << "Server: " << msg.getPayload() << std::endl;
		break;
	case MessageType::GAME_STATE: {
		GameStateData state = msg.parseGameStateData();
		handleGameState(state);
		break;
	}
	case MessageType::ERROR_MSG:
		std::cout << "Server error: " << msg.getPayload() << std::endl;
		break;
	default:
		std::cout << "Received message of type: " << static_cast<int>(msg.getMessageType()) << std::endl;
		break;
	}
}

bool ConsoleClient::connectToServer(const std::string& uri) { 
	try {
		// create new client socket ptr
		client = std::make_unique<WebSocketClient>();
		
		client->setOnConnectCallback([this]() { onConnect(); });
		client->setOnDisconnectCallback([this]() { onDisconnect(); });
		client->setOnMessageCallback([this](const NetworkMessage& msg) { onMessage(msg); });

		client->connect(uri);

		// wait for connection or timeout
		int timeout = timeoutTime; 
		while (!connected && timeout > 0) {
			std::cout << "Waiting for connection..." << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			timeout -= 500;
		}
		if (!connected) {
			std::cerr << "Connection timed out." << std::endl;
			return false;
		}

		//Player pick X or O
		while (true) {
			std::cout << "Choose your side (X/O): ";
			char c;
			if (!(cin >> c)) {
				cin.clear();
				cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				continue;
			}
			c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
			if (c == 'X') {
				player = Player::X;
				break;
			}
			if (c == 'O') {
				player = Player::O;
				break;
			}
			std::cout << "Invalid choice. Enter X or O.\n";
		}

		sendPlayerChoice(player);

		std::cout << "Waiting for game state..." << std::endl;

		// Game loop
		running = true;
		while (running) {
			if (!connected) {
				std::cout << "Server disconnected. Exiting client.\n";
				break;
			}

			bool doTurn = false;
			{
				std::lock_guard<std::mutex> lock(stateMutex);
				doTurn = waitingForTurn;
			}

			if (doTurn) {
				promptAndSendMove();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		// disconnect
		if (connected) {
			client->disconnect();
		}

		// replay
		std::cout << "\nPlay again? (Y/N): ";
		char playAgain;
		if (cin >> playAgain) {
			playAgain = static_cast<char>(toupper(static_cast<unsigned char>(playAgain)));
			if (playAgain == 'Y') {
				// Reset state and reconnect
				{
					std::lock_guard<std::mutex> lock(stateMutex);
					hasState = false;
					waitingForTurn = false;
				}
				return connectToServer(uri);
			}
		}
	}
	catch (const std::exception& e) {
		cerr << "Error setting up connection: " << e.what() << endl;
		return false;
	}
	return true;
}

void ConsoleClient::sendPlayerChoice(Player choice) {
	NetworkMessage choiceMsg = NetworkMessage::createPlayerChoiceMessage(choice);
	client->sendMessage(choiceMsg);
}

void ConsoleClient::promptAndSendMove() {
	int x, y, z;
	bool validInput = false;

	while (!validInput) {
		if (!connected || !running) {
			std::cout << "Connection closed while waiting for input.\n";
			return;
		}

		std::cout << "Enter move (x y z): ";
		cin.clear();
		if (!(cin >> x >> y >> z)) {
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "Invalid input! Please enter three numbers.\n";
			continue;
		}

		if (x < 0 || x > 2 || y < 0 || y > 2 || z < 0 || z > 2) {
			std::cout << "Invalid coordinates! Must be 0-2.\n";
			continue;
		}

		validInput = true;
	}

	std::cout << "Your Move: " << x << " " << y << " " << z << std::endl;

	MoveData move{ player, x, y, z };
	client->sendMessage(NetworkMessage::createMoveMessage(move));
}

void ConsoleClient::handleGameState(const GameStateData& state) {
	bool isGameOver = false;
	Player winner = Player::NONE;

	
	std::lock_guard<std::mutex> lock(stateMutex);
	lastState = state;
	hasState = true;
	waitingForTurn = (!state.game_over && state.current_turn == player);
	isGameOver = state.game_over;
	winner = state.winner;
	

	printBoard(state);

	// end game
	if (isGameOver) {
		if (winner == Player::NONE) {
			std::cout << "Game over: Draw.\n";
		} else {
			std::cout << "Game over: " << (winner == Player::X ? "X" : "O") << " wins.\n";
		}
		running = false;
		return;
	}

	if (waitingForTurn) {
		std::cout << "Your turn.\n";
	} else {
		std::cout << "Waiting for opponent turn...\n";
	}
}

void ConsoleClient::printBoard(const GameStateData& state) const {
	std::cout << "\n";
	for (int i = 0; i < 3; i++) {
		std::cout << "\tz=" << i << "\t";
	}
	std::cout << std::endl;

	for (int row = 2; row >= 0; row--) {
		for (int z = 0; z < 3; z++) {
			if (row == 0) {
				std::cout << "(y" << row << ")[ ";
			}
			else {
				std::cout << "    [ ";
			}

			for (int col = 0; col < 3; col++) {
				char mark;
				switch (state.board[col][row][z]) {
				case Player::X: mark = 'X'; break;
				case Player::O: mark = 'O'; break;
				default: mark = '.'; break;
				}
				std::cout << mark << " ";
			}
			std::cout << "]\t";
		}
		std::cout << "\n";
	}
	for (int i = 0; i < 3; i++) {
		std::cout << "    (x0)\t";
	}
	std::cout << std::endl;
}