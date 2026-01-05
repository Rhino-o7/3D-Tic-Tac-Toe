#pragma once
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <functional>
#include <string>
#include <thread>
#include "network_message.h"

typedef websocketpp::client<websocketpp::config::asio_client> WebSocketClientType;
typedef websocketpp::connection_hdl ConnectionHandle;

class WebSocketClient {
public:
	WebSocketClient();
	~WebSocketClient();

	void connect(const std::string& uri);
	void disconnect();
	void sendMessage(const NetworkMessage& msg);

	bool isConnected() const { return connected; }

	void setOnMessageCallback(std::function<void(const NetworkMessage&)> callback);
	void setOnConnectCallback(std::function<void()> callback);
	void setOnDisconnectCallback(std::function<void()> callback);

	

private:
	void onOpen(ConnectionHandle hdl);
	void onClose(ConnectionHandle hdl);
	void onMessage(ConnectionHandle hdl, WebSocketClientType::message_ptr msg);
	void onFail(ConnectionHandle hdl);

	WebSocketClientType client;
	ConnectionHandle connectionHandle;
	std::thread asioThread;
	bool connected;

	std::function<void(const NetworkMessage&)> onMessageCallback;
	std::function<void()> onConnectCallback;
	std::function<void()> onDisconnectCallback;
};