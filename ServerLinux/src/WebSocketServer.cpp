#include "WebSocketServer.h"

#include <iostream>

// Placeholders
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WebSocketServer::WebSocketServer() : port(9002) {
	server.init_asio();
	server.set_reuse_addr(true);

	server.set_open_handler(bind(&WebSocketServer::onOpen, this, _1));
	server.set_close_handler(bind(&WebSocketServer::onClose, this, _1));
	server.set_message_handler(bind(&WebSocketServer::onMessage, this, _1, _2));
}

WebSocketServer::~WebSocketServer() {
	stop();
}

void WebSocketServer::init(uint16_t port) {
	this->port = port;
	server.listen(port);
	server.start_accept();

	std::cout << "WebSocket server initialized on port " << port << std::endl;
}

void WebSocketServer::run() {
	std::cout << "WebSocket server running..." << std::endl;
	server.run();
}

void WebSocketServer::stop() {
	server.stop_listening();

	for (auto& conn : connections) {
		server.close(conn, websocketpp::close::status::going_away, "Server shutting down");
	}

	server.stop();
}

void WebSocketServer::onOpen(ConnectionHandle hdl) {
	connections.insert(hdl);
	std::cout << "Client connected. Total connections: " << connections.size() << std::endl;

	if (onConnectCallback) {
		onConnectCallback(hdl);
	}
}

void WebSocketServer::onClose(ConnectionHandle hdl) {
	connections.erase(hdl);
	std::cout << "Client disconnected. Total connections: " << connections.size() << std::endl;

	if (onDisconnectCallback) {
		onDisconnectCallback(hdl);
	}
}

void WebSocketServer::onMessage(ConnectionHandle hdl, WebSocketServerType::message_ptr msg) {
	try {
		NetworkMessage networkMsg = NetworkMessage::deserialize(msg->get_payload());

		if (onMessageCallback) {
			onMessageCallback(hdl, networkMsg);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error processing message: " << e.what() << std::endl;
		NetworkMessage errorMsg = NetworkMessage::createErrorMessage(e.what());
		sendMessage(hdl, errorMsg);
	}
}

void WebSocketServer::sendMessage(ConnectionHandle hdl, const NetworkMessage& msg) {
	try {
		server.send(hdl, msg.serialize(), websocketpp::frame::opcode::text);
	}
	catch (const std::exception& e) {
		std::cerr << "Error sending message: " << e.what() << std::endl;
	}
}


// Callbacks
void WebSocketServer::setOnMessageCallback(std::function<void(ConnectionHandle, const NetworkMessage&)> callback) {
	onMessageCallback = callback;
}

void WebSocketServer::setOnConnectCallback(std::function<void(ConnectionHandle)> callback) {
	onConnectCallback = callback;
}

void WebSocketServer::setOnDisconnectCallback(std::function<void(ConnectionHandle)> callback) {
	onDisconnectCallback = callback;
}