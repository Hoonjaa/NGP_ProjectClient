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

    void Update();
    void Disconnect();

    State GetState() const { return state; }
    int GetLastError() const { return lastError; }
    SOCKET GetSocket() const { return serverSocket; }
    bool IsLoggedIn() const { return state == State::Connected && localPlayerId >= 0; }
    short GetPlayerId() const { return localPlayerId; }
    static bool IsValidPlayerName(const std::string& name);


private:
    bool Fail(int error);
    void FlushSendQueue();
    void ReceivePackets();

    void ProcessPendingPackets();
    std::vector<unsigned char> ExtractCompletePackets();
    void DispatchPacket(const unsigned char* packet, std::size_t packetSize);
    
    static constexpr std::size_t maxPendingChats = 64;
    std::size_t sendOffset = 0;
    std::vector<unsigned char> pendingInput;

    SOCKET serverSocket = INVALID_SOCKET;
    bool winsockStarted = false;
    State state = State::Disconnected;
    int lastError = 0;
    short localPlayerId = -1;
    bool loginRequested = false;
    std::chrono::steady_clock::time_point loginStarted;
    std::chrono::steady_clock::time_point connectStarted;
public:
    // true means queued; gameplay requires a successful login response.
    bool C2S_SendLoginInfo(const std::string& name);
    bool C2S_SendChatInfo(const std::string& message, short playerId = -1);
    bool C2S_SendMoveInfo(glm::vec3 Pos, short playerId = -1);

    // 리시브 함수들 다 람다로 해서 Main.cpp에서 정의하는데 이것도 어캐할지 생각해볼 필요는 있을 듯.
    std::function<void(short)> onLoginResult;
    std::function<void(short, const std::string&)> onOtherLogin;
    std::function<void(short, const std::string&)> onChatReceived;
    std::function<void(short, const float x, const float y)> onOtherMoveReceived;
    std::function<void(short, const int)> onHitInfo;

    std::queue<std::vector<unsigned char>> outgoing;
private:
    void S2C_RecvLoginResult(const unsigned char* packet, std::size_t packetSize);
    void S2C_RecvOtherLogin(const unsigned char* packet, std::size_t packetSize);
    void S2C_RecvChatInfo(const unsigned char* packet, std::size_t packetSize);
    void S2C_RecvPlayerHPInfo(const unsigned char* packet, std::size_t packetSize); //피격 당한 플레이어가 HP정보를 받는 함수
    void S2C_RecvWrongWayInfo(const unsigned char* packet, std::size_t packetSize); //잘못된 이동에 대한 정보를 받는 함수
    void S2C_RecvPlayerMoveInfo(const unsigned char* packet, std::size_t packetSize); //다른 클라이언트들의 움직임 정보 받는 함수
    void S2C_RecvBulletInfo(const unsigned char* packet, std::size_t packetSize); //다른 클라이언트들이 총을 발사했다는 정보 받는 함수
    void S2C_RecvPlayerStateInfo(const unsigned char* packet, std::size_t packetSize); //다른 클라이언트의 생존, 죽음, 무적정보 받는 함수
    void S2C_RecvOtherReloading(const unsigned char* packet, std::size_t packetSize); //다른 플레이어 장전 정보 받는 함수
    void S2C_RecvForward(const unsigned char* packet, std::size_t packetSize); //플레이어 방향벡터 정보 받는 함수
    void S2C_GameStart(const unsigned char* packet, std::size_t packetSize); //게임 시작정보 받는 함수
    void S2C_GameEnd(const unsigned char* packet, std::size_t packetSize); //게임 종료 정보 받는 함수
    void S2C_RecvKilllog(const unsigned char* packet, std::size_t packetSize); //누가 어떤 플레이어를 죽였는지에 대한 정보 받는 함수
};
