#pragma once
#include"object.h"

//플레이어와 npc의 부모
class MoveObj :public Object
{
public:
	MoveObj() { 
		
		m_right = Vector3(1.0f, 0.0f, 0.0f);
		m_look = Vector3(0.0f,0.0f, 1.0f);
		m_up = Vector3(0.0f, 1.0f, 0.0f);
	};

	virtual ~MoveObj() = default;

	Vector3 GetOriginX()const { return m_origin_pos; }
	

	float GetDamge()const { return m_damage; }
	float GetHP()const { return m_hp; }
	float GetMaxHP()const { return m_maxhp; }
	

	Vector3 GetRightVec()const { return m_right; }
	Vector3 GetLookVec()const { return m_look; }
	Vector3 GetUpVec()const { return m_up; }
	
	int GetRoomID()const { return m_room_id; }
	bool GetIsActive()const { return m_is_active; }


	void SetIsActive(bool val) { m_is_active = val; }
	void SetRightVec(const Vector3& val) { m_right = val; }
	void SetLookVec(const Vector3& val) { m_look = val; }
	void SetUpVec(const Vector3& val) { m_up = val; }

	void SetDamge(float val) {  m_damage=val; }
	void SetHP(float val)    {  m_hp=val; }
	void SetMaxHP(float val) {  m_maxhp=val; }
	
	void SetRoomID(int val) { m_room_id = val; }

	void SetOriginPos(const Vector3& val) { m_origin_pos = val; }
protected:
	std::atomic_bool m_is_active = true; //죽어있는지 살아있는지
	int m_room_id;
	float m_damage;
	float m_hp, m_maxhp;
	
	Vector3 m_right, m_look, m_up;
	Vector3 m_origin_pos;
};