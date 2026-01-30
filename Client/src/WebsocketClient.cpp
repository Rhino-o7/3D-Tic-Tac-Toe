#include "WebsocketClient.h"
#include <iostream>

#ifndef __EMSCRIPTEN__

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

#else // __EMSCRIPTEN__

#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>

WebSocketClient::WebSocketClient() : socketId(-1), connected(false) {
	std::cout << "WebSocketClient (Emscripten): Initializing..." << std::endl;
}

WebSocketClient::~WebSocketClient() {
	std::cout << "WebSocketClient (Emscripten): Destructor called" << std::endl;
	disconnect();
}

void WebSocketClient::connect(const std::string& uri) {
	if (socketId >= 0) {
		std::cerr << "WebSocketClient: Already connected or connecting" << std::endl;
		return;
	}

	std::cout << "WebSocketClient (Emscripten): Connecting to " << uri << std::endl;

	// Check if WebSocket is supported
	if (!emscripten_websocket_is_supported()) {
		std::cerr << "WebSocketClient: WebSockets are not supported by this browser" << std::endl;
		return;
	}

	// Create WebSocket attributes
	EmscriptenWebSocketCreateAttributes attrs;
	emscripten_websocket_init_create_attributes(&attrs);
	attrs.url = uri.c_str();
	attrs.createOnMainThread = false;

	// Create WebSocket
	socketId = emscripten_websocket_new(&attrs);
	if (socketId < 0) {
		std::cerr << "WebSocketClient: Failed to create WebSocket (error: " << socketId << ")" << std::endl;
		return;
	}

	// Set event handlers
	emscripten_websocket_set_onopen_callback(socketId, this, emscripten_onopen_callback);
	emscripten_websocket_set_onerror_callback(socketId, this, emscripten_onerror_callback);
	emscripten_websocket_set_onclose_callback(socketId, this, emscripten_onclose_callback);
	emscripten_websocket_set_onmessage_callback(socketId, this, emscripten_onmessage_callback);

	std::cout << "WebSocketClient (Emscripten): Connection initiated (socket id: " << socketId << ")" << std::endl;
}

void WebSocketClient::disconnect() {
	if (socketId >= 0) {
		std::cout << "WebSocketClient (Emscripten): Disconnecting..." << std::endl;
		emscripten_websocket_close(socketId, 1000, "Client disconnecting");
		emscripten_websocket_delete(socketId);
		socketId = -1;
		connected = false;
	}
}

void WebSocketClient::sendMessage(const NetworkMessage& msg) {
	if (!connected || socketId < 0) {
		std::cerr << "Cannot send message: not connected" << std::endl;
		return;
	}

	try {
		std::string serialized = msg.serialize();
		std::cout << "Sending message (Emscripten): " << serialized << std::endl;
		
		EMSCRIPTEN_RESULT result = emscripten_websocket_send_utf8_text(socketId, serialized.c_str());
		if (result != EMSCRIPTEN_RESULT_SUCCESS) {
			std::cerr << "Failed to send message (error: " << result << ")" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Send error: " << e.what() << std::endl;
	}
}

void WebSocketClient::setOnMessageCallback(std::function<void(const NetworkMessage&)> callback) {
	onMessageCallback = callback;
}

void WebSocketClient::setOnConnectCallback(std::function<void()> callback) {
	onConnectCallback = callback;
}

void WebSocketClient::setOnDisconnectCallback(std::function<void()> callback) {
	onDisconnectCallback = callback;
}

// Static callback implementations
EM_BOOL WebSocketClient::emscripten_onopen_callback(int eventType, const EmscriptenWebSocketOpenEvent* event, void* userData) {
	WebSocketClient* client = static_cast<WebSocketClient*>(userData);
	client->connected = true;
	std::cout << "WebSocketClient (Emscripten): Connected to server!" << std::endl;

	if (client->onConnectCallback) {
		client->onConnectCallback();
	}

	return EM_TRUE;
}

EM_BOOL WebSocketClient::emscripten_onerror_callback(int eventType, const EmscriptenWebSocketErrorEvent* event, void* userData) {
	std::cerr << "WebSocketClient (Emscripten): WebSocket error occurred" << std::endl;
	return EM_TRUE;
}

EM_BOOL WebSocketClient::emscripten_onclose_callback(int eventType, const EmscriptenWebSocketCloseEvent* event, void* userData) {
	WebSocketClient* client = static_cast<WebSocketClient*>(userData);
	client->connected = false;
	
	std::cout << "WebSocketClient (Emscripten): Disconnected from server (code: " 
	          << event->code << ", reason: " << event->reason << ")" << std::endl;

	if (client->onDisconnectCallback) {
		client->onDisconnectCallback();
	}

	return EM_TRUE;
}

EM_BOOL WebSocketClient::emscripten_onmessage_callback(int eventType, const EmscriptenWebSocketMessageEvent* event, void* userData) {
	WebSocketClient* client = static_cast<WebSocketClient*>(userData);
	
	try {
		if (event->isText) {
			std::string message(reinterpret_cast<const char*>(event->data), event->numBytes);
			std::cout << "WebSocketClient (Emscripten): Received raw message: " << message << std::endl;

			NetworkMessage networkMsg = NetworkMessage::deserialize(message);

			if (client->onMessageCallback) {
				client->onMessageCallback(networkMsg);
			}
		}
		else {
			std::cerr << "Received binary message, but expected text" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Message parse error: " << e.what() << std::endl;
	}

	return EM_TRUE;
}

#endif // __EMSCRIPTEN__

