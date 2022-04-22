#include <include/client_core.h>
#include "message_event_info.h"
#include"server/network_move_object.h"
namespace revive
{

	NpcAttackEventInfo::NpcAttackEventInfo(UINT event_id, const Vec3& target_pos)
		: MessageEventInfo(event_id), m_target_pos(target_pos)

	{
	}
	SignInMessageEventInfo::SignInMessageEventInfo(UINT event_id, char* id, char* pw):
		MessageEventInfo(event_id)
	{
		strcpy_s(m_user_id, id);
		strcpy_s(m_user_pw, pw);

	}
	
	ObjectInfoMessageEventInfo::ObjectInfoMessageEventInfo(UINT event_id, const SPtr<NetworkMoveObj>& other)
		:MessageEventInfo(event_id),m_network_object(other)
	{

		//m_network_object =CreateSPtr<NetworkMoveObj>(other);
	}
	SignUpMessageEventInfo::SignUpMessageEventInfo(UINT event_id, char* id, char* pw):
		MessageEventInfo(event_id)
	{
		strcpy_s(m_user_id, id);
		strcpy_s(m_user_pw, pw);

	}
	MatchingMessageEventInfo::MatchingMessageEventInfo(UINT event_id, int user_num):
		MessageEventInfo(event_id),m_user_num(user_num)
	{
	}
	MoveObjectMessageEventInfo::MoveObjectMessageEventInfo(UINT event_id, const Vec3& pos, const Quaternion& rot)
		: MessageEventInfo(event_id), m_pos(pos),m_rot(rot)
	{
	}
	SendAttackEventInfo::SendAttackEventInfo(UINT event_id)
		: MessageEventInfo(event_id)
	{
	}
	RecvAttackEventInfo::RecvAttackEventInfo(UINT event_id)
		: MessageEventInfo(event_id)
	{
	}
	MatchingMessageOKEventInfo::MatchingMessageOKEventInfo(UINT event_id)
		:MessageEventInfo(event_id)
	{
	}
}
