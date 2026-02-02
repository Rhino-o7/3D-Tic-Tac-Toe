#include "ai.h"
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include "WebSocketServer.h"
#include "GameSession.h"

WebSocketServer wsServer;

std::map<ConnectionHandle, std::shared_ptr<GameSession>, std::owner_less<ConnectionHandle>> activeSessions;
std::mutex sessionsMutex;

static std::shared_ptr<GameSession> GetSession(ConnectionHandle hdl) {
	std::lock_guard<std::mutex> lock(sessionsMutex);
	auto it = activeSessions.find(hdl);
	return it != activeSessions.end() ? it->second : nullptr;
}

void OnClientConnect(ConnectionHandle hdl) {
	std::cout << "New client connected, creating game session..." << std::endl;

	auto session = std::make_shared<GameSession>(wsServer, hdl);
	{
		std::lock_guard<std::mutex> lock(sessionsMutex);
		activeSessions[hdl] = session;
	}

	NetworkMessage welcomeMsg(MessageType::CONNECT, "Welcome to 3D Tic-Tac-Toe! Choose X or O");
	wsServer.SendNetworkMessage(hdl, welcomeMsg);
}

void OnClientDisconnect(ConnectionHandle hdl) {
	std::cout << "Client disconnected, cleaning up session..." << std::endl;

	std::shared_ptr<GameSession> session;
	{
		std::lock_guard<std::mutex> lock(sessionsMutex);
		auto it = activeSessions.find(hdl);
		if (it != activeSessions.end()) {
			session = it->second;
			activeSessions.erase(it);
		}
	}

	if (session) {
		session->HandleDisconnect();
	}
}

void OnClientMessage(ConnectionHandle hdl, const NetworkMessage& msg) {
	auto session = GetSession(hdl);
	if (!session) {
		wsServer.SendNetworkMessage(hdl, NetworkMessage::createErrorMessage("Session not found"));
		return;
	}

	session->HandleMessage(msg);
}

int main() {
	try {
		int port = 0;
		std::cout << "Enter port number: ";
		std::cin >> port;

		if (std::cin.fail() || port < 1 || port > 65535) {
			std::cerr << "Invalid port number. Please enter a value between 1 and 65535." << std::endl;
			return 1;
		}

		wsServer.SetOnConnectCallback(OnClientConnect);
		wsServer.SetOnDisconnectCallback(OnClientDisconnect);
		wsServer.SetOnMessageCallback(OnClientMessage);

		std::cout << "Starting server on port " << port << "..." << std::endl;
		wsServer.Init(port);
		wsServer.Run();
	}
	catch (const std::exception& e) {
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

