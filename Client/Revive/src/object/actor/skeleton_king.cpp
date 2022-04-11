#include <include/client_core.h>
#include "object/actor/skeleton_king.h"
#include <client/object/component/mesh/skeletal_mesh_component.h>
#include <client/object/component/render/box_component.h>
#include "client/input/input.h"

namespace revive
{
	SkeletonKing::SkeletonKing(const std::string& name)
		:Enemy("Contents/TestKing/skeleton_king.rev", name)
	{
	}
	bool SkeletonKing::Initialize()
	{
		bool ret = true;

		ret &= Enemy::Initialize();
		m_skeletal_mesh_component->SetLocalRotation(80.1f, 0.0f, 0.0f);
		ret &= AttachComponent(m_skeletal_mesh_component);

		m_blocking_box_component->SetLocalPosition(Vec3{ 0.0f,110.0f,0.0f });
		m_blocking_box_component->SetExtents(Vec3{ 50.0f,200.0f,50.0f });
		ret &= AttachComponent(m_blocking_box_component);

		SetPosition(Vec3{ 2400.0f,300.0f,4000.0f });

		if (Input::RegisterPressedEvent("Test", { {eKey::kT} },
			[this]()->bool { m_skeletal_mesh_component->SetAnimation(m_animation_name[m_animation_select_num++]);
		if (m_animation_select_num >= m_animation_name.size()) m_animation_select_num = 0; return true; }
			, true, eInputOwnerType::kActor))
			RegisterInputEvent("Test");
		

		return ret;
	}
	void SkeletonKing::Shutdown()
	{
		Enemy::Shutdown();
	}
}