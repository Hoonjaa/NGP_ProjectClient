#pragma once
constexpr short PORT = 9500;
constexpr int MAX_CHAT_MSG_LEN = 200;

enum PACKET_TYPE {
	C2S_LOGIN,
	C2S_MOVEINFO,
	C2S_SHOOTINFO,
	C2S_CHATINFO,
	C2S_FORWARDINFO,
	C2S_RELOADING,

	S2C_LOGINRESULT,
	S2C_OTHERLOGIN,
	S2C_CHAT,
	S2C_KILLLOG,
	S2C_HITINFO,
	S2C_DEATH,
	S2C_REASPAWN,
	S2C_RELEASEINVINCIBILITY,
	S2C_OTHERFORWARD,
	S2C_OTHERMOVE,
	S2C_OTHERSHOOTINFO,
	S2C_BULLETMOVE,
	S2C_BULLETDELETE,
	S2C_OTHERRELOADING,
	S2C_CANTMOVE,
	S2C_GAMESTART,
	S2C_GAMEEND
};
#pragma pack(push, 1) 
struct C2S_Login {	//C2S_LOGIN
	unsigned char size;
	PACKET_TYPE   type;
	char name[10];	// 플레이어 이름
};

struct C2S_MoveInfo {	//C2S_MOVEINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id; // 플레이어 ID
	unsigned char w;
	unsigned char a;
	unsigned char s;
	unsigned char d;
};

struct C2S_ShootInfo {	//C2S_SHOOTINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	float x;		// 발사 위치 X
	float y;		// 발사 위치 Y
	float z;		// 발사 위치 Z
	float dx;		// 발사 방향 X
	float dy;		// 발사 방향 Y
	float dz;		// 발사 방향 Z
};

struct C2S_ChatInfo {	//C2S_CHATINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	char msg[MAX_CHAT_MSG_LEN]; // 채팅 메시지
};

struct C2S_ForwardInfo {	//C2S_FORWARDINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	float pitch;
	float yaw;
};

struct C2S_Reloading {	//C2S_RELOADING
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_LoginResult {	//S2C_LOGINRESULT
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	float x;
	float z;
};

struct S2C_OtherLogin {	//S2C_OTHERLOGIN
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	char name[10];	// 플레이어 이름
	float x;
	float z;
};

struct S2C_Chat {	//S2C_CHAT
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	char msg[MAX_CHAT_MSG_LEN]; // 채팅 메시지
};

struct S2C_KillLog {	//S2C_KILLLOG 
	unsigned char size;
	PACKET_TYPE   type;
	short killer_id;		// 킬한 플레이어 ID
	short victim_id;		// 죽은 플레이어 ID
};

struct S2C_HitInfo {	//S2C_HITINFO
	unsigned char size;
	PACKET_TYPE   type;
	short playerID;		// 플레이어 ID
	int HP;		// 데미지받고 난 후 HP
};

struct S2C_Death {	//S2C_DEATH
	unsigned char size;
	PACKET_TYPE   type;
	short playerID;
	short killerID;	// 죽인 플레이어 ID
};

struct S2C_Respawn {	//S2C_REASPAWN
	unsigned char size;
	PACKET_TYPE   type;
	short playerID;
	float x;	// 리스폰 위치 X
	float z;	// 리스폰 위치 Z
};

struct S2C_ReleaseInvincibility {	//S2C_RELEASEINVINCIBILITY
	unsigned char size;
	PACKET_TYPE   type;
	short playerID;
};

struct S2C_OtherForward {	//S2C_OTHERFORWARD
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	float pitch;
	float yaw;
};

struct S2C_OtherMove {		//S2C_OTHERMOVE
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;		// 플레이어 ID
	float x;				// 플레이어 위치 X
	float z;				// 플레이어 위치 Z
};

struct S2C_OtherShootInfo {	//S2C_OTHERSHOOTINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	float x;
	float y;
	float z;
};

struct S2C_BulletMove {
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	unsigned int index;
	float x;
	float y;
	float z;
};

struct S2C_BulletDelete {
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	unsigned int index;
};

struct S2C_OtherReloading {	//S2C_OTHERRELOADING
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_CantMove {	//S2C_CANTMOVE
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_GameStart {	//S2C_GAMESTART
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_GameEnd {	//S2C_GAMEEND
	unsigned char size;
	PACKET_TYPE   type;
};
#pragma pack(pop)
