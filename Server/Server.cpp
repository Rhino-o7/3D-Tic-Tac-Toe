// Server.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include <iostream>
#include "board.h"
#include "WebSocketServer.h"
#include "GameSession.h"
#include <map>
#include <memory>

std::map<ConnectionHandle, std::unique_ptr<GameSession>, std::owner_less<ConnectionHandle>> activeSessions;
WebSocketServer wsServer;


void onClientConnect(ConnectionHandle hdl) {
	std::cout << "New client connected, creating game session..." << std::endl;
	activeSessions[hdl] = std::make_unique<GameSession>(hdl, &wsServer);
	
	// Send welcome message
	NetworkMessage welcomeMsg(MessageType::CONNECT, "Welcome to 3D Tic-Tac-Toe! Choose X or O");
	wsServer.sendMessage(hdl, welcomeMsg);
}

void onClientDisconnect(ConnectionHandle hdl) {
	std::cout << "Client disconnected, cleaning up session..." << std::endl;
	activeSessions.erase(hdl);
}

void onClientMessage(ConnectionHandle hdl, const NetworkMessage& msg) {
	auto it = activeSessions.find(hdl);
	if (it != activeSessions.end()) {
		it->second->handleMessage(msg);
	}
}

int main()
{
	
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



