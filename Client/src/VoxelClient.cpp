#include "VoxelClient.h"
#include <iostream>
#ifndef __EMSCRIPTEN__
    #include <thread>
#endif
#include "WebsocketClient.h"

VoxelClient::VoxelClient() : m_Player(Player::X), m_Connected(false) {
	m_Client = std::make_unique<WebSocketClient>();
}

void VoxelClient::onConnect()
{
	std::cout << "Connected to server." << std::endl;
	m_Connected = true;
}

void VoxelClient::onDisconnect()
{
	std::cout << "Disconnected from server." << std::endl;
	m_Connected = false;
	{
		std::lock_guard<std::mutex> lock(m_StateMutex);
		m_WaitingForTurn = false;
		m_HasState = false;
	}
	m_Running = false;
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
			std::lock_guard<std::mutex> lock(m_StateMutex);
			if (m_HasState && m_LastState.current_turn == m_Player && !m_LastState.game_over) {
				m_WaitingForTurn = true;
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
	std::lock_guard<std::mutex> lock(m_StateMutex);
	if (m_WaitingForTurn) {
		MoveData move{ m_Player, x, y, z };
		m_Client->SendNetworkMessage(NetworkMessage::createMoveMessage(move));
		m_WaitingForTurn = false;
		return true;
	}
	return false;
}

void VoxelClient::SetPlayerChoice(Player choice)
{
	m_Player = choice;
	m_WaitingForPlayerChoice = false;
	NetworkMessage choiceMsg = NetworkMessage::createPlayerChoiceMessage(m_Player);
	m_Client->SendNetworkMessage(choiceMsg);
}

void VoxelClient::handleGameState(const GameStateData& state)
{
	bool isGameOver = false;
	Player winner = Player::NONE;
	bool stateChanged = false;

	{
		std::lock_guard<std::mutex> lock(m_StateMutex);
		
		if (m_HasState) {
			// Compare boards and track changes
			for (int x = 0; x < 3; ++x) {
				for (int y = 0; y < 3; ++y) {
					for (int z = 0; z < 3; ++z) {
						if (m_LastState.board[x][y][z] != state.board[x][y][z]) {
							m_UpdatedChunks.push_back(glm::vec3(x, y, z));
							stateChanged = true;
						}
					}
				}
			}
		}
		else {
			// mark all chunks as updated
			for (int x = 0; x < 3; ++x) {
				for (int y = 0; y < 3; ++y) {
					for (int z = 0; z < 3; ++z) {
						if (state.board[x][y][z] != Player::NONE) {
							m_UpdatedChunks.push_back(glm::vec3(x, y, z));
							stateChanged = true;
						}
					}
				}
			}
		}
		
		m_LastState = state;
		m_HasState = true;
		m_WaitingForTurn = (!state.game_over && state.current_turn == m_Player);
		isGameOver = state.game_over;
		winner = state.winner;
	}

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
		m_Running = false;
		return;
	}

	if (m_WaitingForTurn) {
		std::cout << "Your turn.\n";
	}
	else {
		std::cout << "Waiting for opponent turn...\n";
	}
}



void VoxelClient::StartGameLoop()
{
#ifndef __EMSCRIPTEN__
    m_Running = true;
    while (m_Running) {
        if (!m_Connected) {
            std::cout << "Server disconnected. Exiting client.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // No longer running
    if (m_Connected) {
        m_Client->Disconnect();
    }
#else
    // Web build
    m_Running = false;
#endif
}

void VoxelClient::StopGameLoop()
{
    m_Running = false;
}

bool VoxelClient::connectToServer(const std::string& uri)
{
    try {
        // Create client socket ptr
        m_Client = std::make_unique<WebSocketClient>();

        m_Client->SetOnConnectCallback([this]() { onConnect(); });
        m_Client->SetOnDisconnectCallback([this]() { onDisconnect(); });
        m_Client->SetOnMessageCallback([this](const NetworkMessage& msg) { onMessage(msg); });

        m_Client->Connect(uri);

#ifdef __EMSCRIPTEN__
        // Connection is async so return true to indicate connection initiated
        std::cout << "Web build: Connection initiated asynchronously..." << std::endl;
        return true;
#else
        // Wait for connection or timeout
        int timeout = m_TimeoutTime;
        while (!m_Connected && timeout > 0) {
            std::cout << "Waiting for connection..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            timeout -= 500;
        }
        if (!m_Connected) {
            std::cerr << "Connection timed out." << std::endl;
            return false;
        }

        std::cout << "Connected. Ready to set player choice from UI..." << std::endl;
        return true;
#endif
    }
    catch (const std::exception& e) {
        std::cerr << "Error setting up connection: " << e.what() << std::endl;
        return false;
    }
}
