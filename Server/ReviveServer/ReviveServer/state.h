#pragma once


enum class EVENT_TYPE { 
	EVENT_NPC_MOVE, 
	EVENT_PLAYER_HILL, 
	EVENT_NPC_ATTACK, 
	EVENT_REGEN 
};

enum class COMP_OP {
	OP_RECV, 
	OP_SEND, 
	OP_ACCEPT, 
	OP_NPC_MOVE, 
	OP_PLAYER_MOVE, 
	OP_PLAYER_HILL,
	OP_TRACKING_PLAYER, 
	OP_NPC_ATTACK,
	OP_REGEN
};

enum class STATE 
{ 
	ST_FREE, 
	ST_ACCEPT, 
	ST_INGAME 
};