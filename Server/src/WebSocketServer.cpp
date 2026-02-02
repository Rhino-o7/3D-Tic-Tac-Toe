#include "WebSocketServer.h"

#include <iostream>

// Placeholders
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WebSocketServer::WebSocketServer() : m_Port(9002) {
	m_Server.init_asio();
	m_Server.set_reuse_addr(true);

	m_Server.set_open_handler(bind(&WebSocketServer::OnOpen, this, _1));
	m_Server.set_close_handler(bind(&WebSocketServer::OnClose, this, _1));
	m_Server.set_message_handler(bind(&WebSocketServer::OnMessage, this, _1, _2));
}

WebSocketServer::~WebSocketServer() {
	Stop();
}

void WebSocketServer::Init(uint16_t port) {
	this->m_Port = port;
	m_Server.listen(port);
	m_Server.start_accept();

	std::cout << "WebSocket server initialized on port " << port << std::endl;
}

void WebSocketServer::Run() {
	std::cout << "WebSocket server running..." << std::endl;
	m_Server.run();
}

void WebSocketServer::Stop() {
	m_Server.stop_listening();

	for (auto& conn : m_Connections) {
		m_Server.close(conn, websocketpp::close::status::going_away, "Server shutting down");
	}

	m_Server.stop();
}

void WebSocketServer::OnOpen(ConnectionHandle hdl) {
	m_Connections.insert(hdl);
	std::cout << "Client connected. Total connections: " << m_Connections.size() << std::endl;

	if (OnConnectCallback) {
		OnConnectCallback(hdl);
	}
}

void WebSocketServer::OnClose(ConnectionHandle hdl) {
	m_Connections.erase(hdl);
	std::cout << "Client disconnected. Total connections: " << m_Connections.size() << std::endl;

	if (OnDisconnectCallback) {
		OnDisconnectCallback(hdl);
	}
}

void WebSocketServer::OnMessage(ConnectionHandle hdl, WebSocketServerType::message_ptr msg) {
	try {
		NetworkMessage networkMsg = NetworkMessage::deserialize(msg->get_payload());

		if (OnMessageCallback) {
			OnMessageCallback(hdl, networkMsg);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error processing message: " << e.what() << std::endl;
		NetworkMessage errorMsg = NetworkMessage::createErrorMessage(e.what());
		SendNetworkMessage(hdl, errorMsg);
	}
}

void WebSocketServer::SendNetworkMessage(ConnectionHandle hdl, const NetworkMessage& msg) {
	try {
		m_Server.send(hdl, msg.serialize(), websocketpp::frame::opcode::text);
	}
	catch (const std::exception& e) {
		std::cerr << "Error sending message: " << e.what() << std::endl;
	}
}


// Callbacks
void WebSocketServer::SetOnMessageCallback(std::function<void(ConnectionHandle, const NetworkMessage&)> callback) {
	OnMessageCallback = callback;
}

void WebSocketServer::SetOnConnectCallback(std::function<void(ConnectionHandle)> callback) {
	OnConnectCallback = callback;
}

void WebSocketServer::SetOnDisconnectCallback(std::function<void(ConnectionHandle)> callback) {
	OnDisconnectCallback = callback;
}