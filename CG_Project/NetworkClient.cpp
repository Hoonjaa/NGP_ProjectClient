#include "NetworkClient.h"
#include <cstring>
#include <algorithm>
#include <type_traits>

static_assert(sizeof(C2S_ChatInfo) <= 255, "Chat packet must fit the size field.");
static_assert(sizeof(S2C_Chat) <= 255, "Chat packet must fit the size field.");

NetworkClient::~NetworkClient()
{
    Disconnect();
}

bool NetworkClient::Connect(const char* serverIPv4, unsigned short port)
{
    Disconnect();

    WSADATA data{};
    const int startupError = WSAStartup(MAKEWORD(2, 2), &data);
    if (startupError != 0) return Fail(startupError);
    winsockStarted = true;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (!serverIPv4 || port == 0 ||
        InetPtonA(AF_INET, serverIPv4, &address.sin_addr) != 1) {
        return Fail(WSAEINVAL);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) return Fail(WSAGetLastError());

    // A slow or unreachable server must not stop the window's event loop.
    u_long nonblocking = 1;
    if (ioctlsocket(serverSocket, FIONBIO, &nonblocking) == SOCKET_ERROR) {
        return Fail(WSAGetLastError());
    }

    connectStarted = std::chrono::steady_clock::now();
    if (connect(serverSocket, reinterpret_cast<const sockaddr*>(&address),
                sizeof(address)) == 0) {
        state = State::Connected;
        return true;
    }

    const int error = WSAGetLastError();
    if (error != WSAEWOULDBLOCK) return Fail(error);
    state = State::Connecting;
    return true;
}

void NetworkClient::Update()
{
    if (state == State::Connected) {
        FlushSendQueue();
        if (state == State::Connected) ReceivePackets();
        return;
    }
    if (state != State::Connecting) return;

    fd_set writable, errors;
    FD_ZERO(&writable);
    FD_ZERO(&errors);
    FD_SET(serverSocket, &writable);
    FD_SET(serverSocket, &errors);
    timeval noWait{};
    const int ready = select(0, nullptr, &writable, &errors, &noWait);
    if (ready == SOCKET_ERROR) {
        Fail(WSAGetLastError());
        return;
    }

    if (ready > 0) {
        int error = 0;
        int errorSize = sizeof(error);
        if (getsockopt(serverSocket, SOL_SOCKET, SO_ERROR,
                       reinterpret_cast<char*>(&error), &errorSize) == SOCKET_ERROR) {
            Fail(WSAGetLastError());
        }
        else if (error != 0) {
            Fail(error);
        }
        else if (FD_ISSET(serverSocket, &errors)) {
            Fail(WSAECONNABORTED);
        }
        else if (FD_ISSET(serverSocket, &writable)) {
            state = State::Connected;
        }
        return;
    }

    constexpr auto timeout = std::chrono::seconds(5);
    if (std::chrono::steady_clock::now() - connectStarted >= timeout) {
        Fail(WSAETIMEDOUT);
    }
}

bool NetworkClient::SendChat(const std::string& message, short playerId)
{
    if (state != State::Connected) {
        lastError = WSAENOTCONN;
        return false;
    }
    if (message.empty() || message.find_first_not_of(' ') == std::string::npos ||
        message.size() >= MAX_CHAT_MSG_LEN || message.find('\0') != std::string::npos) {
        lastError = WSAEINVAL;
        return false;
    }
    if (outgoingChats.size() >= maxPendingChats) {
        lastError = WSAENOBUFS;
        return false;
    }

    C2S_ChatInfo packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_CHATINFO;
    packet.player_id = playerId;
    // Zero initialization leaves a terminator and clears unused packet bytes.
    std::memcpy(packet.msg, message.data(), message.size());
    outgoingChats.push_back(packet);
    lastError = 0;
    return true;
}

void NetworkClient::FlushSendQueue()
{
    if (outgoingChats.empty()) return;

    // One attempt per Update keeps rendering responsive even with a full queue.
    const auto& packet = outgoingChats.front();
    const int written = send(serverSocket,
        reinterpret_cast<const char*>(&packet) + sendOffset,
        static_cast<int>(sizeof(packet) - sendOffset), 0);
    if (written == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) Fail(error);
        return;
    }
    if (written == 0) {
        Fail(WSAECONNRESET);
        return;
    }

    sendOffset += static_cast<std::size_t>(written);
    if (sendOffset == sizeof(packet)) {
        outgoingChats.pop_front();
        sendOffset = 0;
    }
}

void NetworkClient::ReceivePackets()
{
    // One nonblocking read per Update bounds work even if the server keeps sending.
    unsigned char buffer[4096];
    const int received = recv(serverSocket, reinterpret_cast<char*>(buffer),
        static_cast<int>(sizeof(buffer)), 0);
    if (received == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) Fail(error);
        return;
    }
    if (received == 0) {
        Fail(WSAECONNRESET);
        return;
    }

    pendingInput.insert(pendingInput.end(), buffer, buffer + received);
    constexpr std::size_t headerSize = sizeof(unsigned char) + sizeof(PACKET_TYPE);
    std::size_t consumed = 0;
    // Dispatch after parsing so callbacks never run while the input buffer is in use.
    std::vector<S2C_Chat> chats;
    while (consumed < pendingInput.size()) {
        const std::size_t packetSize = pendingInput[consumed];
        if (packetSize < headerSize) {
            Fail(WSAEINVAL);
            return;
        }
        if (pendingInput.size() - consumed < packetSize) break;

        const unsigned char* packet = pendingInput.data() + consumed;
        std::underlying_type_t<PACKET_TYPE> type{};
        std::memcpy(&type, packet + sizeof(unsigned char), sizeof(type));
        if (type == S2C_CHAT) {
            if (packetSize != sizeof(S2C_Chat)) {
                Fail(WSAEINVAL);
                return;
            }
            S2C_Chat chat{};
            std::memcpy(&chat, packet, sizeof(chat));
            chats.push_back(chat);
        }
        consumed += packetSize;
    }
    pendingInput.erase(pendingInput.begin(), pendingInput.begin() + consumed);

    for (const auto& chat : chats) {
        const char* end = std::find(chat.msg, chat.msg + MAX_CHAT_MSG_LEN, '\0');
        const std::string message(chat.msg, end);
        if (!message.empty() && onChatReceived) onChatReceived(chat.player_id, message);
        if (state != State::Connected) return;
    }
}

void NetworkClient::Disconnect()
{
    outgoingChats.clear();
    sendOffset = 0;
    pendingInput.clear();
    if (serverSocket != INVALID_SOCKET) {
        if (state == State::Connected) shutdown(serverSocket, SD_BOTH);
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    if (winsockStarted) {
        WSACleanup();
        winsockStarted = false;
    }
    state = State::Disconnected;
    lastError = 0;
}

bool NetworkClient::Fail(int error)
{
    Disconnect();
    lastError = error;
    state = State::Failed;
    return false;
}
