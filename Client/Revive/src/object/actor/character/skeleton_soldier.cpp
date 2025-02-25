#include <include/client_core.h>
#include <client/object/component/mesh/skeletal_mesh_component.h>
#include <client/object/component/render/sphere_component.h>
#include <client/object/component/render/box_component.h>
#include <client/input/input.h>
#include "object/actor/character/revive_player.h"
#include "object/actor/character/skeleton_soldier.h"
#include "object/actor/gameplaymechanics/base.h"
#include "object/actor/projectile/stone.h"

#include <client/object/component/render/widget_component.h>
#include "object/ui/enemy_info_ui_layer.h"

namespace revive
{
	SkeletonSoldier::SkeletonSoldier(const std::string& name)
		:Enemy("Contents/skeleton_soldier.rev",name)
	{
	}
	bool SkeletonSoldier::Initialize()
	{
		bool ret = true;

		ret &= Enemy::Initialize();
		ret &= AttachComponent(m_skeletal_mesh_component);
		m_skeletal_mesh_component->SetLocalRotation(math::ToRadian(-90.0f), 0.0f, 0.0f);
		m_skeletal_mesh_component->SetLocalScale(100.f);

		m_skeletal_mesh_component->AddNotify("death end", "death", 109,
			[this]() { m_is_disappearing = true; /*Destory되기 까지 Count를 시작한다*/ });
		m_skeletal_mesh_component->AddNotify("hit end", "hit", 10,
			[this]() { m_skeletal_mesh_component->SetAnimation("run"); /*히트 후에 재생할 애니메이션*/});
		m_skeletal_mesh_component->AddNotify("fire", "attack", 12, [this]() { Fire();});
		m_skeletal_mesh_component->AddNotify("attack end", "attack", 20,
			[this]() { 
			m_is_attacking = false; m_is_fire = false; m_skeletal_mesh_component->SetAnimation("run"); /*공격 후에 재생할 애니메이션*/});
		ret &= SetCollisionComponent();

		m_hp = 10;
		//SetPosition(Vec3{ 2400.0f,300.0f,3500.0f });
		SetScale(0.7f); 

		//Test용
		/*if (Input::RegisterPressedEvent(m_name + " Test", { {eKey::kR} },
			[this]()->bool { m_skeletal_mesh_component->SetAnimation(m_animation_name[m_animation_select_num++]);
		if (m_animation_select_num >= m_animation_name.size()) m_animation_select_num = 0; return true; }
			, true, eInputOwnerType::kActor))
			RegisterInputEvent(m_name + " Test");*/
		
		m_widget_component->SetLocalPosition(Vec3(0.0f, 200.0f, 0.0f));

		return ret;
	}

	void SkeletonSoldier::Fire()
	{
		if (m_is_fire == false)// 안해주면 돌멩이 2연사함
		{
			m_is_fire = true;
			const auto& stone = CreateSPtr<Stone>();
			stone->SetPosition(m_skeletal_mesh_component->GetBoneWorldPosition("mount0"));
			stone->SetRotation(m_skeletal_mesh_component->GetLocalRotation() * GetRotation());
			stone->SetBlockingSphereRadius(10.f);
			Vec3 direction = vec3::Normalize(m_target_position - stone->GetPosition());//플레이어 위치조정 필요
			stone->SetVelocity(direction);
			stone->SetCollisionInfo(true, "stone", { "player hit","base"}, true);
			stone->SetOnCollisionResponse([stone,this](const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor,
				const SPtr<SceneComponent>& other_component)
			{
				//이게 피격
				stone->SetCollisionInfo(false, false, false);
				//LOG_INFO(component->GetName() + " " + other_actor->GetName() + " " + other_component->GetName());
				const auto& player = std::dynamic_pointer_cast<DefaultPlayer>(other_actor);
				if (player != nullptr)
				{
					float player_hp = player->GetHP();
					if (player_hp > 0)
					{
							//플레이어 피격했다고 서버에 알리기
							player->Hit(0,m_network_id);		
					}
				}
				else
				{
					const auto& base = std::dynamic_pointer_cast<Base>(other_actor);
					if (base != nullptr)
					{
						float base_hp = base->GetHP();
						if (base_hp > 0)
						{
							//기지 피격했다고 서버에 알리기
							base->SetHP(base_hp);
						}
					}
				}
				//LOG_INFO("충돌 부위 :" + other_component->GetName());
				stone->SetActorState(eActorState::kDead);
				
			});
			SpawnActor(stone);
		}
	}

	void SkeletonSoldier::Shutdown()
	{
		Enemy::Shutdown();
	}

	void SkeletonSoldier::Update(float delta_time)
	{
		Enemy::Update(delta_time);
	}

	bool SkeletonSoldier::SetCollisionComponent()
	{
		bool ret = true;

		m_blocking_sphere->SetLocalPosition(Vec3{ 0.0f,m_blocking_sphere->GetExtents().y,0.0f }); //x,y,z 상관없음
		m_blocking_sphere->SetCollisionInfo(false, false, "enemy", { "wall" }, true);
		m_blocking_sphere->OnCollisionResponse([this](const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor,
			const SPtr<SceneComponent>& other_component) {
			FixYPosition();
			//LOG_INFO(GetName() + ": sphere component {0} Enemy Position {1} Extents {2}", m_blocking_sphere->GetWorldPosition(), this->GetPosition(), m_blocking_sphere->GetExtents());
		});
		//ret &= AttachComponent(m_blocking_sphere);

		//멀티에서만 사용
		m_blocking_box->SetExtents(Vec3{25.f,90.f,45.f});
		m_blocking_box->SetLocalPosition(Vec3{ 0.0f,m_blocking_box->GetExtents().y,0.0f });
		m_blocking_box->SetCollisionInfo(false, false, "enemy", { "wall" }, true);
		m_blocking_box->OnCollisionResponse([this](const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor,
			const SPtr<SceneComponent>& other_component) {
			//LOG_INFO(GetName() + ": Box component {0} Enemy Position {1} Extents {2}", m_blocking_box->GetWorldPosition(), this->GetPosition(), m_blocking_box->GetExtents());
		});
		//ret &= AttachComponent(m_blocking_box);

		//공격 범위 구체
		m_agro_sphere->OnCollisionResponse([this](const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor,
			const SPtr<SceneComponent>& other_component) {
			float distance = (GetPosition() - other_actor->GetPosition()).Length();
			SetRotation(FindLookAtRotation(GetPosition(), other_actor->GetPosition()));
			if (distance < 1700.f)
			{
				const auto& player = std::dynamic_pointer_cast<DefaultPlayer>(other_actor);
				if (player != nullptr)
				{
					m_target_position = other_actor->GetPosition() + Vec3{0.f,100.f,0.f};
					if (player->IsDying() == false) Attack();
				}
				else
				{
					const auto& base = std::dynamic_pointer_cast<Base>(other_actor);
					if (base != nullptr)
					{
						m_target_position = other_actor->GetPosition() + Vec3{ 0.f,200.f,0.f };

						float base_hp = base->GetHP();
						if (base_hp > 0)
							Attack();
					}
				}
			}
			//LOG_INFO("시야 범위에 인식된 플레이어 :" + other_actor->GetName());
		});

		//Hit Box
		m_hit_box->SetExtents(Vec3{ 30.0f,100.f,40.f });
		m_hit_box->SetLocalPosition(Vec3{ 0.0f,100.f,0.f });
		m_hit_box->SetCollisionInfo(true, false, "enemy hit", { "bullet" }, false);
		ret &= AttachComponent(m_hit_box);
		return ret;
	}
	void SkeletonSoldier::Attack()
	{
		if (m_is_attacking == false)
		{
			Enemy::Attack();
		}
	}
}