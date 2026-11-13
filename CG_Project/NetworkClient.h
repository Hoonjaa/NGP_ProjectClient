#pragma once

#include "pch.h"
#include "Protocol.h"
#include <chrono>
#include <deque>
#include <functional>
#include <string>
#include <vector>

// Owns one TCP connection. Call all methods on the GLUT thread, and Update each frame.
class NetworkClient
{
public:
    enum class State { Disconnected, Connecting, Connected, Failed };

    NetworkClient() = default;
    ~NetworkClient();
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    // true means the attempt started (or already completed); inspect GetState.
    bool Connect(const char* serverIPv4, unsigned short port);
    // true means queued, not acknowledged by the server. -1 means ID not assigned yet.
    bool SendChat(const std::string& message, short playerId = -1);
    // Invoked by Update on the GLUT thread when a complete S2C_CHAT arrives.
    std::function<void(short, const std::string&)> onChatReceived;
    void Update();
    void Disconnect();

    State GetState() const { return state; }
    int GetLastError() const { return lastError; }
    SOCKET GetSocket() const { return serverSocket; }

private:
    bool Fail(int error);
    void FlushSendQueue();
    void ReceivePackets();

    static constexpr std::size_t maxPendingChats = 64;
    std::deque<C2S_ChatInfo> outgoingChats;
    std::size_t sendOffset = 0;
    std::vector<unsigned char> pendingInput;

    SOCKET serverSocket = INVALID_SOCKET;
    bool winsockStarted = false;
    State state = State::Disconnected;
    int lastError = 0;
    std::chrono::steady_clock::time_point connectStarted;
};
