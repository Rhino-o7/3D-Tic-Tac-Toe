#include "VoxelClient.h"
#include <iostream>
#include <limits>
#include <thread>
#include "WebsocketClient.h"

VoxelClient::VoxelClient() : player(Player::X), connected(false) {
	client = std::make_unique<WebSocketClient>();
}

void VoxelClient::onConnect()
{
	std::cout << "Connected to server." << std::endl;
	connected = true;
}

void VoxelClient::onDisconnect()
{
	std::cout << "Disconnected from server." << std::endl;
	connected = false;
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		waitingForTurn = false;
		hasState = false;
	}
	running = false;
}

void VoxelClient::onMessage(const NetworkMessage& msg)
{
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
		{
			std::lock_guard<std::mutex> lock(stateMutex);
			if (hasState && lastState.current_turn == player && !lastState.game_over) {
				waitingForTurn = true;
			}
		}
		break;
	default:
		std::cout << "Received message of type: " << static_cast<int>(msg.getMessageType()) << std::endl;
		break;
	}
}
void printBoard(const GameStateData& state)  {
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

bool VoxelClient::SendMove(int x, int y, int z) {
	std::lock_guard<std::mutex> lock(stateMutex);
	if (waitingForTurn) {
		MoveData move{ player, x, y, z };
		client->sendMessage(NetworkMessage::createMoveMessage(move));
		waitingForTurn = false;
		return true;
	}
	return false;
}

void VoxelClient::SetPlayerChoice(Player choice)
{
	player = choice;
	waitingForPlayerChoice = false;
	NetworkMessage choiceMsg = NetworkMessage::createPlayerChoiceMessage(player);
	client->sendMessage(choiceMsg);
}

void VoxelClient::handleGameState(const GameStateData& state)
{
	bool isGameOver = false;
	Player winner = Player::NONE;
	bool stateChanged = false;

	{
		std::lock_guard<std::mutex> lock(stateMutex);
		
		// Track which chunks changed by comparing old and new state
		// Don't clear here - let the rendering code clear after processing
		if (hasState) {
			// Compare boards and track changes
			for (int x = 0; x < 3; ++x) {
				for (int y = 0; y < 3; ++y) {
					for (int z = 0; z < 3; ++z) {
						if (lastState.board[x][y][z] != state.board[x][y][z]) {
							updatedChunks.push_back(glm::vec3(x, y, z));
							stateChanged = true;
						}
					}
				}
			}
		}
		else {
			// First state - mark all chunks as updated
			for (int x = 0; x < 3; ++x) {
				for (int y = 0; y < 3; ++y) {
					for (int z = 0; z < 3; ++z) {
						if (state.board[x][y][z] != Player::NONE) {
							updatedChunks.push_back(glm::vec3(x, y, z));
							stateChanged = true;
						}
					}
				}
			}
		}
		
		lastState = state;
		hasState = true;
		waitingForTurn = (!state.game_over && state.current_turn == player);
		isGameOver = state.game_over;
		winner = state.winner;
	}

	// Notify callback if state changed (outside the lock to avoid deadlock)
	if (stateChanged && onStateChangeCallback) {
		onStateChangeCallback();
	}

	// End game
	if (isGameOver) {
		if (winner == Player::NONE) {
			std::cout << "Game over: Draw.\n";
		}
		else {
			std::cout << "Game over: " << (winner == Player::X ? "X" : "O") << " wins.\n";
		}
		running = false;
		return;
	}

	if (waitingForTurn) {
		std::cout << "Your turn.\n";
	}
	else {
		std::cout << "Waiting for opponent turn...\n";
	}
}



void VoxelClient::StartGameLoop()
{
	running = true;
	while (running) {
		if (!connected) {
			std::cout << "Server disconnected. Exiting client.\n";
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// No longer running
	if (connected) {
		client->disconnect();
	}
}

void VoxelClient::StopGameLoop()
{
	running = false;
}

bool VoxelClient::connectToServer(const std::string & uri)
{
	try {
		// Create new client socket ptr
		client = std::make_unique<WebSocketClient>();

		client->setOnConnectCallback([this]() { onConnect(); });
		client->setOnDisconnectCallback([this]() { onDisconnect(); });
		client->setOnMessageCallback([this](const NetworkMessage& msg) { onMessage(msg); });

		client->connect(uri);

		// Wait for connection or timeout
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

		std::cout << "Connected. Ready to set player choice from UI..." << std::endl;
		return true;
	}
	catch (const std::exception& e) {
		std::cerr << "Error setting up connection: " << e.what() << std::endl;
		return false;
	}
}
