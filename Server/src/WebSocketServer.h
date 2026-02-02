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

	void Init(uint16_t port);
	void Run();
	void Stop();
	void SendNetworkMessage(ConnectionHandle hdl, const NetworkMessage& msg);

	// Callbacks
	void SetOnMessageCallback(std::function<void(ConnectionHandle, const NetworkMessage&)> callback);
	void SetOnConnectCallback(std::function<void(ConnectionHandle)> callback);
	void SetOnDisconnectCallback(std::function<void(ConnectionHandle)> callback);

private:
	void OnOpen(ConnectionHandle hdl);
	void OnClose(ConnectionHandle hdl);
	void OnMessage(ConnectionHandle hdl, WebSocketServerType::message_ptr msg);

	WebSocketServerType m_Server;
	uint16_t m_Port;
	std::set<ConnectionHandle, std::owner_less<ConnectionHandle>> m_Connections;

	std::function<void(ConnectionHandle, const NetworkMessage&)> OnMessageCallback;
	std::function<void(ConnectionHandle)> OnConnectCallback;
	std::function<void(ConnectionHandle)> OnDisconnectCallback;

	
};