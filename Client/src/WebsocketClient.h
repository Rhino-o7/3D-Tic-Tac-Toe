#pragma once

#include <functional>
#include <string>
#include "network_message.h"

#ifndef __EMSCRIPTEN__

    #include <websocketpp/config/asio_no_tls_client.hpp>
    #include <websocketpp/client.hpp>
    #include <thread>

    typedef websocketpp::client<websocketpp::config::asio_client> WebSocketClientType;
    typedef websocketpp::connection_hdl ConnectionHandle;

    class WebSocketClient {
    public:
        WebSocketClient();
        ~WebSocketClient();

        void Connect(const std::string& uri);
        void Disconnect();
        void SendNetworkMessage(const NetworkMessage& msg);

        bool isConnected() const { return m_Connected; }

        void SetOnMessageCallback(std::function<void(const NetworkMessage&)> callback);
        void SetOnConnectCallback(std::function<void()> callback);
        void SetOnDisconnectCallback(std::function<void()> callback);

    private:
        void OnOpen(ConnectionHandle hdl);
        void OnClose(ConnectionHandle hdl);
        void OnMessage(ConnectionHandle hdl, WebSocketClientType::message_ptr msg);
        void OnFail(ConnectionHandle hdl);

        WebSocketClientType m_Client;
        ConnectionHandle m_ConnectionHandle;
        std::thread m_AsioThread;
        bool m_Connected = false;

        std::function<void(const NetworkMessage&)> onMessageCallback;
        std::function<void()> onConnectCallback;
        std::function<void()> onDisconnectCallback;
    };

#else   // __EMSCRIPTEN__

    #include <emscripten/websocket.h>

    // Emscripten WebSocket implementation using browser WebSocket API
    class WebSocketClient {
    public:
        WebSocketClient();
        ~WebSocketClient();

        void Connect(const std::string& uri);
        void Disconnect();
        void SendNetworkMessage(const NetworkMessage& msg);

        bool isConnected() const { return connected; }

        void SetOnMessageCallback(std::function<void(const NetworkMessage&)> callback);
        void SetOnConnectCallback(std::function<void()> callback);
        void SetOnDisconnectCallback(std::function<void()> callback);

    private:
        int socketId = -1;
        bool connected = false;

        std::function<void(const NetworkMessage&)> onMessageCallback;
        std::function<void()> onConnectCallback;
        std::function<void()> onDisconnectCallback;

        // Static callbacks for Emscripten C API must match Emscripten's expected signatures
        static EM_BOOL emscripten_onopen_callback(int eventType, const EmscriptenWebSocketOpenEvent* event, void* userData);
        static EM_BOOL emscripten_onerror_callback(int eventType, const EmscriptenWebSocketErrorEvent* event, void* userData);
        static EM_BOOL emscripten_onclose_callback(int eventType, const EmscriptenWebSocketCloseEvent* event, void* userData);
        static EM_BOOL emscripten_onmessage_callback(int eventType, const EmscriptenWebSocketMessageEvent* event, void* userData);
    };

#endif