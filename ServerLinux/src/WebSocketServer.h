#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <memory>
#include <functional>
#include "network_message.h"

typedef websocketpp::server<websocketpp::config::asio> WebSocketServerType;
typedef websocketpp::connection_hdl ConnectionHandle;

class GameSession;

class WebSocketServer {
public:
	WebSocketServer();
	~WebSocketServer();

	void init(uint16_t port);
	void run();
	void stop();
	void sendMessage(ConnectionHandle hdl, const NetworkMessage& msg);

	// Callbacks
	void setOnMessageCallback(std::function<void(ConnectionHandle, const NetworkMessage&)> callback);
	void setOnConnectCallback(std::function<void(ConnectionHandle)> callback);
	void setOnDisconnectCallback(std::function<void(ConnectionHandle)> callback);

private:
	void onOpen(ConnectionHandle hdl);
	void onClose(ConnectionHandle hdl);
	void onMessage(ConnectionHandle hdl, WebSocketServerType::message_ptr msg);

	WebSocketServerType server;
	std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> connections;

	std::function<void(ConnectionHandle, const NetworkMessage&)> onMessageCallback;
	std::function<void(ConnectionHandle)> onConnectCallback;
	std::function<void(ConnectionHandle)> onDisconnectCallback;

	uint16_t port;
};