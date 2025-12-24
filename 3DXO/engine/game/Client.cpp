//#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define BOOST_ALL_NO_LIB

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
        
typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

// Simple WebSocket client class
class GameClient {
private:
    client ws_client;
    websocketpp::connection_hdl connection;
    std::string server_uri;

public:
    GameClient(const std::string& uri) : server_uri(uri) {
        // Initialize the client
        ws_client.init_asio();
        
        // Set logging to only show errors
        ws_client.set_error_channels(websocketpp::log::elevel::all);
        ws_client.set_access_channels(websocketpp::log::alevel::none);
        
        // Bind event handlers
        ws_client.set_open_handler(bind(&GameClient::on_open, this, _1));
        ws_client.set_message_handler(bind(&GameClient::on_message, this, _1, _2));
        ws_client.set_close_handler(bind(&GameClient::on_close, this, _1));
        ws_client.set_fail_handler(bind(&GameClient::on_fail, this, _1));
    }

    // Connect to the server
    void connect() {
        websocketpp::lib::error_code ec;
        client::connection_ptr con = ws_client.get_connection(server_uri, ec);
        
        if (ec) {
            std::cout << "Connection failed: " << ec.message() << std::endl;
            return;
        }
        
        connection = con->get_handle();
        ws_client.connect(con);
    }

    // Start the client (blocking call)
    void run() {
        ws_client.run();
    }

    // Send a message to the server
    void send_message(const std::string& message) {
        websocketpp::lib::error_code ec;
        ws_client.send(connection, message, websocketpp::frame::opcode::text, ec);
        
        if (ec) {
            std::cout << "Send failed: " << ec.message() << std::endl;
        }
    }

    // Close the connection
    void close() {
        websocketpp::lib::error_code ec;
        ws_client.close(connection, websocketpp::close::status::normal, "Client closing", ec);
        
        if (ec) {
            std::cout << "Close failed: " << ec.message() << std::endl;
        }
    }

private:
    // Called when connection is successfully opened
    void on_open(websocketpp::connection_hdl hdl) {
        std::cout << "Connected to server!" << std::endl;
        
        // Example: Send a message when connected
        send_message("Hello from client!");
    }

    // Called when a message is received from the server
    void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
        std::cout << "Received: " << msg->get_payload() << std::endl;
    }

    // Called when connection is closed
    void on_close(websocketpp::connection_hdl hdl) {
        std::cout << "Connection closed" << std::endl;
    }

    // Called when connection fails
    void on_fail(websocketpp::connection_hdl hdl) {
        std::cout << "Connection failed" << std::endl;
    }
};

// Main function to test the client
int main() {
    std::cout << "=== WebSocket Client Test ===" << std::endl;
    std::cout << "Attempting to connect to ws://localhost:9002..." << std::endl;
    std::cout << "(Make sure your server is running first!)" << std::endl;
    std::cout << std::endl;

    try {
        // Create client instance
        GameClient game_client("ws://localhost:9002");

        // Connect to the server
        game_client.connect();

        // Run in a separate thread so we can demonstrate non-blocking behavior
        std::thread client_thread([&game_client]() {
            game_client.run();
        });

        // Wait a bit to see if connection succeeds or fails
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Demonstrate sending additional messages (if connected)
        std::cout << "\nSending test messages..." << std::endl;
        game_client.send_message("Test message 1");
        game_client.send_message("Test message 2");
        game_client.send_message("MOVE:X5Y3Z2");

        // Keep running for a bit to receive any responses
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Close the connection
        std::cout << "\nClosing connection..." << std::endl;
        game_client.close();

        // Wait for the thread to finish
        client_thread.join();

        std::cout << "\nTest complete!" << std::endl;

    }
    catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}