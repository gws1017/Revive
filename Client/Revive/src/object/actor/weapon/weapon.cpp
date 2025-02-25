#include <include/client_core.h>
#include <client/object/component/mesh/static_mesh_component.h>
#include <client/object/component/mesh/skeletal_mesh_component.h>
#include <client/physics/collision/collisioner/collisioner.h>
#include <client/object/component/render/box_component.h>
#include <client/input/input.h>
#include "object/actor/weapon/weapon.h"

namespace revive
{
	Weapon::Weapon(const std::string& path, const std::string& name)
		:Actor(eMobilityState::kMovable,name)
		,m_mesh_path(path)
	{
		m_static_mesh_component = CreateSPtr<StaticMeshComponent>();
		m_box_component = CreateSPtr<BoxComponent>();
	}

	bool Weapon::Initialize()
	{
		bool ret = m_static_mesh_component->SetMesh(m_mesh_path);
		ret &= AttachComponent(m_static_mesh_component);
		//ret &= AttachComponent(m_box_component);
		return ret;

	}


	void Weapon::Update(float delta_time)
	{
		if (m_attached_actor.expired() == false)
		{
			m_static_mesh_component->SetLocalPosition(m_position_offset);
			Mat4 socket_world_matrix = m_attached_skeletal_mesh_component.lock()->GetBoneWorldMatrix(m_socket_name);
			SetPosition(Vec3{ socket_world_matrix._41,socket_world_matrix._42,socket_world_matrix._43 });
			SetRotation(
				quat::CreateQuaternionFromRollPitchYaw(math::ToRadian(m_rotation_offset.x), math::ToRadian(m_rotation_offset.y), math::ToRadian(m_rotation_offset.z))/*quat::CreateQuaternionFromAxis(vec3::AXIS_X,math::ToRadian(70.f))*///������ ��
				* m_attached_skeletal_mesh_component.lock()->GetBoneWorldRotation(m_socket_name)
			);
		}
	}

	//void const Weapon::GetSocketMatrix(Mat4& out_matrix)
	//{
	//	const auto& skeletal_mesh_component = m_attached_skeletal_mesh_component.lock();
	//	out_matrix = skeletal_mesh_component->GetSocketWorldMatrix(m_socket_name);
	//	
	//	//LOG_INFO(out_matrix);
	//}

	void Weapon::SetAttachedActor(const SPtr<Actor> actor, const SPtr<SkeletalMeshComponent> skeletal_mesh_component)
	{
		m_attached_actor = actor;
		m_attached_skeletal_mesh_component = skeletal_mesh_component;
	}

	void Weapon::SetCollisionInfo(bool is_collision, bool is_blocking, bool generate_collision_event)
	{
		m_box_component->SetCollisionInfo(is_collision, is_blocking, generate_collision_event);
	}

	const CollisionInfo& Weapon::GetCollisionInfo() const
	{
		return m_box_component->GetCollisioner()->GetCollisionInfo();
	}

	void Weapon::Shutdown()
	{
		m_static_mesh_component = nullptr;
		m_box_component = nullptr;
	}

	//void Weapon::SetSocket()
	//{
	//	const auto& character = m_attached_actor.lock();
	//	Mat4 m_character_matrix = mat4::CreateTranslation(character->GetPosition());
	//	m_character_matrix *= mat4::CreateRotationFromQuaternion(character->GetW);
	//	Mat4 socket_world_matrix = socket_matrix;
	//	//socket_world_matrix *= actor_matrix;

	//	m_static_mesh_component->SetLocalPosition(Vec3{ socket_world_matrix._41, socket_world_matrix._42, socket_world_matrix._43 });
	//	LOG_INFO("{0}", m_static_mesh_component->GetLocalPosition());
	//	Quaternion q;
	//	//XMStoreFloat4(&q, XMQuaternionRotationMatrix(XMLoadFloat4x4(&socket_world_matrix)));
	//	//m_static_mesh_component->SetLocalRotation(q);
	//	//LOG_INFO(m_static_mesh_component->GetWorldMatrix());
	//}

}