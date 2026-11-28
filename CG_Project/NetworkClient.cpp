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
        if (state == State::Connected && loginRequested && !IsLoggedIn() &&
            std::chrono::steady_clock::now() - loginStarted >= std::chrono::seconds(5)) {
            Fail(WSAETIMEDOUT);
        }
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

void NetworkClient::FlushSendQueue()
{
    //if (outgoing.empty()) return;

    //// One attempt per Update keeps rendering responsive even with a full queue.
    //const auto& packet = outgoing.front();
    //const int written = send(serverSocket,
    //    reinterpret_cast<const char*>(packet.data()) + sendOffset,
    //    static_cast<int>(packet.size() - sendOffset), 0);
    //if (written == SOCKET_ERROR) {
    //    const int error = WSAGetLastError();
    //    if (error != WSAEWOULDBLOCK) Fail(error);
    //    return;
    //}
    //if (written == 0) {
    //    Fail(WSAECONNRESET);
    //    return;
    //}

    //sendOffset += static_cast<std::size_t>(written);
    //if (sendOffset == packet.size()) {
    //    outgoing.pop();
    //    sendOffset = 0;
    //}

    constexpr int maxSendAttempts = 8;

    for (int attempt = 0;
        attempt < maxSendAttempts && !outgoing.empty();
        ++attempt) {
        const auto& packet = outgoing.front();
        const int written = send(
            serverSocket,
            reinterpret_cast<const char*>(packet.data() + sendOffset),
            static_cast<int>(packet.size() - sendOffset),
            0);

        if (written == SOCKET_ERROR) {
            const int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                Fail(error);
            }
            return;
        }

        if (written == 0) {
            Fail(WSAECONNRESET);
            return;
        }

        sendOffset += static_cast<std::size_t>(written);
        if (sendOffset == packet.size()) {
            outgoing.pop();
            sendOffset = 0;
        }
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
    ProcessPendingPackets();
}

void NetworkClient::ProcessPendingPackets()
{
    // Own the packet bytes locally since a handler can disconnect and clear pendingInput.
    const auto packets = ExtractCompletePackets();
    std::size_t offset = 0;
    while (offset < packets.size() && state == State::Connected) {
        const std::size_t packetSize = packets[offset];
        DispatchPacket(packets.data() + offset, packetSize);
        offset += packetSize;
    }
}

std::vector<unsigned char> NetworkClient::ExtractCompletePackets()
{
    constexpr std::size_t headerSize = sizeof(unsigned char) + sizeof(PACKET_TYPE);
    std::size_t consumed = 0;
    while (consumed < pendingInput.size()) {
        const std::size_t packetSize = pendingInput[consumed];
        if (packetSize < headerSize) {
            Fail(WSAEINVAL);
            return {};
        }
        if (packetSize > pendingInput.size() - consumed) break;
        consumed += packetSize;
    }

    const auto end = pendingInput.begin() + consumed;
    std::vector<unsigned char> packets(pendingInput.begin(), end);
    pendingInput.erase(pendingInput.begin(), end);
    return packets;
}

void NetworkClient::DispatchPacket(const unsigned char* packet, std::size_t packetSize)
{
    std::underlying_type_t<PACKET_TYPE> type{};
    std::memcpy(&type, packet + sizeof(unsigned char), sizeof(type));

    switch (type) {
    case S2C_LOGINRESULT:           S2C_RecvLoginResult(packet, packetSize);        break;
    case S2C_OTHERLOGIN:            S2C_RecvOtherLogin(packet, packetSize);         break;
    case S2C_OTHERLOGOUT:           S2C_RecvOtherLogout(packet, packetSize);        break;
    case S2C_CHAT:                  S2C_RecvChatInfo(packet, packetSize);           break;
    case S2C_KILLLOG:               S2C_RecvKilllog(packet, packetSize);            break;
    case S2C_HITINFO:               S2C_RecvPlayerHPInfo(packet,packetSize);        break;
	case S2C_DEATH:                 S2C_RecvDeathInfo(packet, packetSize);    break;
	case S2C_REASPAWN:              S2C_RecvRespawnInfo(packet, packetSize);    break;
	case S2C_RELEASEINVINCIBILITY:  S2C_ReleaseInvincibilityInfo(packet, packetSize);    break;
	case S2C_OTHERFORWARD:          S2C_RecvForward(packet, packetSize);            break;
    case S2C_OTHERMOVE:             S2C_RecvPlayerMoveInfo(packet, packetSize);     break;
    case S2C_OTHERSHOOTINFO:        S2C_RecvOtherShootInfo(packet, packetSize);         break;
    case S2C_BULLETMOVE:            S2C_RecvBulletMoveInfo(packet, packetSize);     break;
    case S2C_BULLETDELETE:          S2C_RecvBulletDeleteInfo(packet, packetSize);   break;
    case S2C_OTHERRELOADING:        S2C_RecvOtherReloading(packet, packetSize);     break;
    case S2C_CANTMOVE:              S2C_RecvWrongWayInfo(packet, packetSize);       break;
    case S2C_GAMESTART:             S2C_GameStart(packet, packetSize);              break;
    case S2C_GAMEEND:               S2C_GameEnd(packet,packetSize);                 break;
    default:
        break;
    }
}

void NetworkClient::Disconnect()
{
    outgoing = {};
    sendOffset = 0;
    pendingInput.clear();
    localPlayerId = -1;
    loginRequested = false;
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
///////////////////////////////////////////////////////////////////////////////////////////
bool NetworkClient::IsValidPlayerName(const std::string& name)
{
    if (name.empty() || name.size() >= sizeof(C2S_Login{}.name) ||
        name.find_first_not_of(' ') == std::string::npos) return false;
    return std::all_of(name.begin(), name.end(), [](unsigned char c) {
        return c >= 32 && c <= 126;
    });
}

bool NetworkClient::C2S_SendLoginInfo(const std::string& name)
{
    if (state != State::Connected) {
        lastError = WSAENOTCONN;
        return false;
    }
    if (loginRequested || !IsValidPlayerName(name)) {
        lastError = WSAEINVAL;
        return false;
    }

    C2S_Login packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_LOGIN;
    std::memcpy(packet.name, name.data(), name.size());
    std::vector<unsigned char> bytes(sizeof(packet));
    std::memcpy(bytes.data(), &packet, sizeof(packet));
    outgoing.push(bytes);
    loginRequested = true;
    loginStarted = std::chrono::steady_clock::now();
    lastError = 0;
    return true;
}

bool NetworkClient::C2S_SendChatInfo(const std::string& message, short playerId)
{
    if (!IsLoggedIn()) {
        lastError = WSAENOTCONN;
        return false;
    }
    if (playerId >= 0 && playerId != localPlayerId) {
        lastError = WSAEINVAL;
        return false;
    }
    if (message.empty() || message.find_first_not_of(' ') == std::string::npos ||
        message.size() >= MAX_CHAT_MSG_LEN || message.find('\0') != std::string::npos) {
        lastError = WSAEINVAL;
        return false;
    }
    if (outgoing.size() >= maxPendingChats) {
        lastError = WSAENOBUFS;
        return false;
    }

    C2S_ChatInfo packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_CHATINFO;
    packet.player_id = localPlayerId;
    // Zero initialization leaves a terminator and clears unused packet bytes.
    std::memcpy(packet.msg, message.data(), message.size());

    std::vector<unsigned char> responsepacket(sizeof(packet));
    std::memcpy(responsepacket.data(), &packet, sizeof(packet));

    outgoing.push(responsepacket);
    lastError = 0;
    return true;
}

bool NetworkClient::C2S_SendMoveInfo(short playerId, bool w, bool a, bool s, bool d)
{
    if (!IsLoggedIn()) {
        lastError = WSAENOTCONN;
        return false;
    }
    if (playerId >= 0 && playerId != localPlayerId) {
        lastError = WSAEINVAL;
        return false;
    }
    C2S_MoveInfo packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_MOVEINFO;
    packet.player_id = localPlayerId;
    packet.w = w ? 1 : 0;
    packet.a = a ? 1 : 0;
    packet.s = s ? 1 : 0;
    packet.d = d ? 1 : 0;
    std::vector<unsigned char> responsepacket(sizeof(packet));
    std::memcpy(responsepacket.data(), &packet, sizeof(packet));
    outgoing.push(responsepacket);
    lastError = 0;
    return true;
}

bool NetworkClient::C2S_SendForwardInfo(float pitch, float yaw)
{
    if (!IsLoggedIn()) {
        lastError = WSAENOTCONN;
        return false;
    }

    if (hasQueuedForward && pitch == lastQueuedPitch && yaw == lastQueuedYaw) {
        lastError = 0;
        return false;  // 각도가 같아서 큐에 추가할 필요가 없음
    }

	C2S_ForwardInfo packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_FORWARDINFO;
    packet.player_id = localPlayerId;
    packet.pitch = pitch;
    packet.yaw = yaw;

    std::vector<unsigned char> responsepacket(sizeof(packet));
    std::memcpy(responsepacket.data(), &packet, sizeof(packet));
    outgoing.push(responsepacket);

    hasQueuedForward = true;
    lastQueuedPitch = pitch;
    lastQueuedYaw = yaw;
    lastError = 0;
	return true;
}

bool NetworkClient::C2S_SendShootInfo(const glm::vec3 position, const glm::vec3 dir, short playerId)
{
    if (!IsLoggedIn()) {
        lastError = WSAENOTCONN;
        return false;
    }
    if (playerId >= 0 && playerId != localPlayerId) {
        lastError = WSAEINVAL;
        return false;
    }
    C2S_ShootInfo packet{};
    packet.size = static_cast<unsigned char>(sizeof(packet));
    packet.type = C2S_SHOOTINFO;
    packet.player_id = localPlayerId;
    packet.x = position.x;
    packet.y = position.y;
    packet.z = position.z;
    packet.dx = dir.x;
    packet.dy = dir.y;
    packet.dz = dir.z;
    std::vector<unsigned char> responsepacket(sizeof(packet));
    std::memcpy(responsepacket.data(), &packet, sizeof(packet));
    outgoing.push(responsepacket);
    lastError = 0;
    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////
void NetworkClient::S2C_RecvLoginResult(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_LoginResult) || !loginRequested || IsLoggedIn()) {
        Fail(WSAEINVAL);
        return;
    }
    S2C_LoginResult result{};
    std::memcpy(&result, packet, sizeof(result));
    if (result.player_id < 0 || result.player_id >= 3) {
        Fail(WSAEINVAL);
        return;
    }
    localPlayerId = result.player_id;
    if (onLoginResult) onLoginResult(localPlayerId, result.x, result.z);
}

void NetworkClient::S2C_RecvOtherLogin(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_OtherLogin) || !IsLoggedIn()) {
        Fail(WSAEINVAL);
        return;
    }
    S2C_OtherLogin other{};
    std::memcpy(&other, packet, sizeof(other));
    if (other.player_id < 0 || other.player_id >= 3 || other.player_id == localPlayerId) {
        Fail(WSAEINVAL);
        return;
    }
    const char* begin = other.name;
    const char* end = std::find(begin, begin + sizeof(other.name), '\0');
    if (onOtherLogin) onOtherLogin(other.player_id, std::string(begin, end), other.x, other.z);
}

void NetworkClient::S2C_RecvOtherLogout(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_OtherLogout) || !IsLoggedIn()) {
        Fail(WSAEINVAL);
        return;
    }

    S2C_OtherLogout logout{};
    std::memcpy(&logout, packet, sizeof(logout));
    if (!vaildPlayerID(logout.player_id) || logout.player_id == localPlayerId) {
        Fail(WSAEINVAL);
        return;
    }

    if (onOtherLogout) onOtherLogout(logout.player_id);
}

void NetworkClient::S2C_RecvChatInfo(const unsigned char* packet, std::size_t packetSize)
{
	// std::cout << "S2C_RecvChatInfo" << std::endl;
    if (packetSize != sizeof(S2C_Chat)) {
        Fail(WSAEINVAL);
        return;
    }

    S2C_Chat chat{};
    std::memcpy(&chat, packet, sizeof(chat));

    const char* begin = chat.msg;
    const char* end = std::find(begin, begin + MAX_CHAT_MSG_LEN, '\0');
    const std::string message(begin, end);
    if (!message.empty() && onChatReceived) onChatReceived(chat.player_id, message);
}

//피격 당한 플레이어가 HP정보를 받는 함수
void NetworkClient::S2C_RecvPlayerHPInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_HitInfo)) {
        Fail(WSAEINVAL);
        return;
    }

	S2C_HitInfo hitinfo{};
	std::memcpy(&hitinfo, packet, sizeof(hitinfo));
    if (hitinfo.playerID <0 || hitinfo.playerID >= 3) {
        Fail(WSAEINVAL);
        return;
    }
	if (onHitInfo) onHitInfo(hitinfo.playerID, hitinfo.HP);
}

//잘못된 이동에 대한 정보를 받는 함수
void NetworkClient::S2C_RecvWrongWayInfo(const unsigned char* packet, std::size_t packetSize)
{
}

//다른 클라이언트들의 움직임 정보 받는 함수
void NetworkClient::S2C_RecvPlayerMoveInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_OtherMove)) {
        Fail(WSAEINVAL);
        return;
	}

    S2C_OtherMove move;
	std::memcpy(&move, packet, sizeof(move));
	if (IsLoggedIn() && onOtherMoveReceived) onOtherMoveReceived(move.player_id, move.x, move.z);
}

//플레이어 방향벡터 정보 받는 함수
void NetworkClient::S2C_RecvForward(const unsigned char* packet, std::size_t packetSize)
{
	printf("S2C_RecvForward called with packetSize: %zu\n", packetSize);
    if(packetSize != sizeof(S2C_OtherForward)) {
        Fail(WSAEINVAL);
        return;
	}
	S2C_OtherForward forward;
	std::memcpy(&forward, packet, sizeof(forward));
	if (IsLoggedIn() && onOtherForwardReceived) onOtherForwardReceived(forward.player_id, forward.pitch, forward.yaw);
}


//다른 클라이언트들이 총을 발사했다는 정보 받는 함수
void NetworkClient::S2C_RecvOtherShootInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_OtherShootInfo)) {
        Fail(WSAEINVAL);
        return;
    }
    S2C_OtherShootInfo shootInfo;
    std::memcpy(&shootInfo, packet, sizeof(shootInfo));
    glm::vec3 pos = { shootInfo.x, shootInfo.y, shootInfo.z };
    if (IsLoggedIn() && onOtherShootInfoReceived) onOtherShootInfoReceived(shootInfo.player_id, pos);
}

void NetworkClient::S2C_RecvBulletMoveInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_BulletMove)) {
        Fail(WSAEINVAL);
        return;
    }
    S2C_BulletMove bulletInfo;
    std::memcpy(&bulletInfo, packet, sizeof(bulletInfo));
    glm::vec3 pos = { bulletInfo.x, bulletInfo.y, bulletInfo.z };
    if (IsLoggedIn() && onBulletMoveInfoReceived) onBulletMoveInfoReceived(bulletInfo.player_id, bulletInfo.index, pos);
}

void NetworkClient::S2C_RecvBulletDeleteInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_BulletDelete)) {
        Fail(WSAEINVAL);
        return;
    }
    S2C_BulletDelete bulletInfo;
    std::memcpy(&bulletInfo, packet, sizeof(bulletInfo));
    if (IsLoggedIn() && onBulletDeleteInfoReceived) onBulletDeleteInfoReceived(bulletInfo.player_id, bulletInfo.index);
}

void NetworkClient::S2C_RecvDeathInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_Death)) {
        Fail(WSAEINVAL);
        return;
    }
	S2C_Death death{};
    std::memcpy(&death, packet, sizeof(death));
    if (vaildPlayerID(death.playerID) && vaildPlayerID(death.killerID) && onDeathInfo) {
		onDeathInfo(death.playerID, death.killerID);
    }
}

void NetworkClient::S2C_RecvRespawnInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_Respawn)) {
        Fail(WSAEINVAL);
        return;
    }
	S2C_Respawn respawn{};
	std::memcpy(&respawn, packet, sizeof(respawn));
    if (vaildPlayerID(respawn.playerID) && onRespawnInfo) {
		onRespawnInfo(respawn.playerID, respawn.x, respawn.z);
    }
}

void NetworkClient::S2C_ReleaseInvincibilityInfo(const unsigned char* packet, std::size_t packetSize)
{
    if (packetSize != sizeof(S2C_ReleaseInvincibility)) {
        Fail(WSAEINVAL);
        return;
    }
	S2C_ReleaseInvincibility release{};
	std::memcpy(&release, packet, sizeof(release)); 
    if (vaildPlayerID(release.playerID) && onInvincibilityReleaseInfo) {
		onInvincibilityReleaseInfo(release.playerID);
    }
}

//다른 플레이어 장전 정보 받는 함수
void NetworkClient::S2C_RecvOtherReloading(const unsigned char* packet, std::size_t packetSize)
{
}

//게임 시작정보 받는 함수
void NetworkClient::S2C_GameStart(const unsigned char* packet, std::size_t packetSize)
{
}

//게임 종료 정보 받는 함수
void NetworkClient::S2C_GameEnd(const unsigned char* packet, std::size_t packetSize)
{
}

//누가 어떤 플레이어를 죽였는지에 대한 정보 받는 함수
void NetworkClient::S2C_RecvKilllog(const unsigned char* packet, std::size_t packetSize)
{
}