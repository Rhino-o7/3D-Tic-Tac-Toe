#include "WebsocketClient.h"
#include <iostream>

// placeholders 
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WebSocketClient::WebSocketClient() : connected(false) {
	std::cout << "WebSocketClient: Initializing..." << std::endl;

	try {
		client.init_asio();

		client.clear_access_channels(websocketpp::log::alevel::all);
		client.clear_error_channels(websocketpp::log::elevel::all);
		client.set_access_channels(websocketpp::log::alevel::connect | websocketpp::log::alevel::disconnect);

		client.set_open_handler(bind(&WebSocketClient::onOpen, this, _1));
		client.set_close_handler(bind(&WebSocketClient::onClose, this, _1));
		client.set_message_handler(bind(&WebSocketClient::onMessage, this, _1, _2));
		client.set_fail_handler(bind(&WebSocketClient::onFail, this, _1));

		std::cout << "WebSocketClient: Initialized successfully" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "WebSocketClient initialization error: " << e.what() << std::endl;
		throw;
	}
}

WebSocketClient::~WebSocketClient() {
	std::cout << "WebSocketClient: Destructor called" << std::endl;
	disconnect();
}

void WebSocketClient::connect(const std::string& uri) {
	try {
		std::cout << "WebSocketClient: Attempting to connect to " << uri << std::endl;
		//check for error
		websocketpp::lib::error_code ec;
		WebSocketClientType::connection_ptr con = client.get_connection(uri, ec);

		if (ec) {
			std::cerr << "Connection error: " << ec.message() << std::endl;
			return;
		}

		connectionHandle = con->get_handle();
		client.connect(con);

		// Start ASIO in thread
		asioThread = std::thread([this]() {
			std::cout << "ASIO thread started" << std::endl;
			try {
				client.run();
				std::cout << "ASIO thread ended" << std::endl;
			}
			catch (const std::exception& e) {
				std::cerr << "ASIO thread error: " << e.what() << std::endl;
			}
			});

		std::cout << "WebSocketClient: Connection initiated" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Connection exception: " << e.what() << std::endl;
	}
}

void WebSocketClient::disconnect() {
	if (connected) {
		std::cout << "WebSocketClient: Disconnecting..." << std::endl;

		websocketpp::lib::error_code ec;
		client.close(connectionHandle, websocketpp::close::status::normal, "Client disconnecting", ec);

		//error
		if (ec) {
			std::cerr << "Disconnect error: " << ec.message() << std::endl;
		}

		connected = false;
	}

	client.stop();

	// end thread
	if (asioThread.joinable()) {
		asioThread.join();
	}
}

void WebSocketClient::sendMessage(const NetworkMessage& msg) {
	if (!connected) {
		std::cerr << "Cannot send message: not connected" << std::endl;
		return;
	}

	try {
		std::string serialized = msg.serialize();
		std::cout << "Sending message: " << serialized << std::endl;
		client.send(connectionHandle, serialized, websocketpp::frame::opcode::text);
	}
	catch (const std::exception& e) {
		std::cerr << "Send error: " << e.what() << std::endl;
	}
}



void WebSocketClient::onOpen(ConnectionHandle hdl) {
	connected = true;
	std::cout << "WebSocketClient: Connected to server!" << std::endl;

	if (onConnectCallback) {
		onConnectCallback();
	}
}

void WebSocketClient::onClose(ConnectionHandle hdl) {
	connected = false;
	std::cout << "WebSocketClient: Disconnected from server" << std::endl;

	if (onDisconnectCallback) {
		onDisconnectCallback();
	}
}

void WebSocketClient::onMessage(ConnectionHandle hdl, WebSocketClientType::message_ptr msg) {
	try {
		std::cout << "WebSocketClient: Received raw message: " << msg->get_payload() << std::endl;

		NetworkMessage networkMsg = NetworkMessage::deserialize(msg->get_payload());

		if (onMessageCallback) {
			onMessageCallback(networkMsg);
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Message parse error: " << e.what() << std::endl;
	}
}

void WebSocketClient::onFail(ConnectionHandle hdl) {
	std::cerr << "WebSocketClient: Connection failed" << std::endl;

	WebSocketClientType::connection_ptr con = client.get_con_from_hdl(hdl);
	std::cerr << "Fail reason: " << con->get_ec().message() << std::endl;

	connected = false;

	
	if (onDisconnectCallback) {
		onDisconnectCallback();
	}
}
// Set Callbacks
void WebSocketClient::setOnMessageCallback(std::function<void(const NetworkMessage&)> callback) {
	onMessageCallback = callback;
}

void WebSocketClient::setOnConnectCallback(std::function<void()> callback) {
	onConnectCallback = callback;
}

void WebSocketClient::setOnDisconnectCallback(std::function<void()> callback) {
	onDisconnectCallback = callback;
}

