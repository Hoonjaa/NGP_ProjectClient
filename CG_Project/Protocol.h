#pragma once
constexpr short PORT = 9500;
constexpr int MAX_CHAT_MSG_LEN = 200;

enum PACKET_TYPE {
	C2S_MOVEINFO,
	C2S_SHOOTINFO,
	C2S_CHATINFO,
	C2S_FORWARDINFO,
	C2S_RELOADING,

	S2C_CHAT,
	S2C_KILLLOG,
	S2C_HITINFO,
	S2C_DEATH,
	S2C_REASPAWN,
	S2C_RELEASEINVINCIBILITY,
	S2C_OTHERFORWARD,
	S2C_OTHERFORWARD,
	S2C_OTHERMOVE,
	S2C_OTHERBULLET,
	S2C_OTHERRELOADING,
	S2C_CANTMOVE,
	S2C_GAMESTART,
	S2C_GAMEEND
};
#pragma pack(push, 1) 
struct C2S_MoveInfo {	//C2S_MOVEINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	short x;
	short y;
};

struct C2S_ShootInfo {	//C2S_SHOOTINFO
	unsigned char size;
	PACKET_TYPE   type;
};

struct C2S_ChatInfo {	//C2S_CHATINFO
	unsigned char size;
	PACKET_TYPE   type;
	short player_id;
	char msg[MAX_CHAT_MSG_LEN];
};

struct C2S_ForwardInfo {	//C2S_FORWARDINFO
	unsigned char size;
	PACKET_TYPE   type;
};

struct C2S_Reloading {	//C2S_RELOADING
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_Chat {	//S2C_CHAT
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_KillLog {	//S2C_KILLLOG 
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_HitInfo {	//S2C_HITINFO
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_Death {	//S2C_DEATH
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_Respawn {	//S2C_REASPAWN
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_ReleaseInvincibility {	//S2C_RELEASEINVINCIBILITY
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_OtherForward {	//S2C_OTHERFORWARD
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_OtherMove {		//S2C_OTHERMOVE
	unsigned char size;
	PACKET_TYPE   type;
};

struct S2C_OtherBullet {	//S2C_OTHERBULLET
	unsigned char size;
	PACKET_TYPE   type;
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
