#include "GameSession.h"

GameSession::GameSession(WebSocketServer& server, ConnectionHandle clientHandle)
	: m_Server(server), m_Client(clientHandle) {
}

void GameSession::HandleMessage(const NetworkMessage& msg) {
	std::lock_guard<std::mutex> lock(m_SessionMutex);

	switch (msg.getMessageType()) {
	case MessageType::PLAYER_CHOICE:
		HandlePlayerChoice(msg.parsePlayerChoice());
		break;
	case MessageType::MOVE:
		HandleMove(msg.parseMoveData());
		break;
	default:
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("Unsupported message type"));
		break;
	}
}

void GameSession::HandleDisconnect() {
	std::lock_guard<std::mutex> lock(m_SessionMutex);
}

void GameSession::HandlePlayerChoice(Player choice) { // Sets the X and O of Player and AI
	if (choice == Player::NONE) {
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("Invalid player choice"));
		return;
	}


	m_ClientPlayer = choice;
	m_AIPlayer = (choice == Player::X) ? Player::O : Player::X;

	m_Game = std::make_unique<Game>(m_ClientPlayer);
	
	
	m_AI = std::make_unique<AI>(m_AIPlayer, &m_Game->getBoard(), m_AIDifficulty);

	// Check who goes first
	if (m_Game->getCurrentTurn() == m_AIPlayer) {
		MakeAiTurn();
	} else {
		SendGameState();
	}

}

void GameSession::HandleMove(const MoveData& move) {
	// Check for errors
	if (!m_Game) {
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("Game not initialized; choose a player first."));
		return;
	}
	if (move.player != m_ClientPlayer || m_Game->getCurrentTurn() != m_ClientPlayer) {
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("Error! Not client turn"));
		return;
	}

	// Check for move error and apply move
	const bool applied = m_Game->takeTurn(move.player, move.x, move.y, move.z);
	if (!applied) {
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("Invalid move"));
		return;
	}

	SendGameState();

	// Ai turn
	if (!m_Game->isGameOver() && m_Game->getCurrentTurn() == m_AIPlayer) {
		MakeAiTurn();
	}
}

void GameSession::MakeAiTurn() {
	if (!m_Game || !m_AI) {
		return;
	}

	// apply ai best move
	auto [x, y, z] = m_AI->GetBestMove();
	const bool applied = m_Game->takeTurn(m_AIPlayer, x, y, z);
	if (!applied) {
		m_Server.SendNetworkMessage(m_Client, NetworkMessage::createErrorMessage("AI failed to place move"));
		return;
	}

	SendGameState();
}

void GameSession::SendGameState() {
	if (!m_Game) {
		return;
	}

	GameStateData state{};
	for (int x = 0; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 0; z < 3; ++z) {
				state.board[x][y][z] = m_Game->getBoard().getCell(x, y, z);
			}
		}
	}

	state.current_turn = m_Game->getCurrentTurn();
	state.game_over = m_Game->isGameOver();
	state.winner = m_Game->checkWin();
	

	m_Server.SendNetworkMessage(m_Client, NetworkMessage::createGameStateMessage(state));
}