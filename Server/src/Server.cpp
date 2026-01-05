#include <game.h>
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

static std::shared_ptr<GameSession> getSession(ConnectionHandle hdl) {
	std::lock_guard<std::mutex> lock(sessionsMutex);
	auto it = activeSessions.find(hdl);
	return it != activeSessions.end() ? it->second : nullptr;
}

void onClientConnect(ConnectionHandle hdl) {
	std::cout << "New client connected, creating game session..." << std::endl;

	auto session = std::make_shared<GameSession>(wsServer, hdl);
	{
		std::lock_guard<std::mutex> lock(sessionsMutex);
		activeSessions[hdl] = session;
	}

	NetworkMessage welcomeMsg(MessageType::CONNECT, "Welcome to 3D Tic-Tac-Toe! Choose X or O");
	wsServer.sendMessage(hdl, welcomeMsg);
}

void onClientDisconnect(ConnectionHandle hdl) {
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
		session->handleDisconnect();
	}
}

void onClientMessage(ConnectionHandle hdl, const NetworkMessage& msg) {
	auto session = getSession(hdl);
	if (!session) {
		wsServer.sendMessage(hdl, NetworkMessage::createErrorMessage("Session not found"));
		return;
	}

	session->handleMessage(msg);
}

int main() {
	try {
		wsServer.setOnConnectCallback(onClientConnect);
		wsServer.setOnDisconnectCallback(onClientDisconnect);
		wsServer.setOnMessageCallback(onClientMessage);

		wsServer.init(9002);
		wsServer.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

