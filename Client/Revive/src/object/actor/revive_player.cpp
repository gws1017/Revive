#include <include/client_core.h>
#include <client/object/component/core/scene_component.h>
#include <client/object/component/util/simple_movement_component.h>
#include <client/object/component/mesh/static_mesh_component.h>
#include <client/object/component/mesh/skeletal_mesh_component.h>
#include <client/object/component/render/sphere_component.h>
#include <client/object/component/render/box_component.h>
#include <client/object/actor/core/actor.h>
#include <client/input/input.h>
#include "object/component/follow_camera.h"
#include "object/statemachine/state_machine.h"
#include "object/actor/revive_player.h"

namespace revive
{
	RevivePlayer::RevivePlayer(const std::string& name)
		: Pawn(name)
	{
		m_camera_component = CreateSPtr<FollowCamera>("Follow Camera", eCameraUsage::kBasic);
		m_movement_component = CreateSPtr<SimpleMovementComponent>();
		m_skeletal_mesh_component = CreateSPtr<SkeletalMeshComponent>();
		m_sphere_component = CreateSPtr<SphereComponent>(30.0f);
		m_player_fsm = CreateSPtr<PlayerFSM>();
		m_mesh_path = "Contents/violet.rev";
	}

	bool RevivePlayer::Initialize()
	{
		bool ret = true;

		ret &= Pawn::Initialize();
		m_movement_component->SetMaxSpeed(300.f);// 타일 크기 300 * 0.75(tile/s)  = 225 (근데 너무느리다)
		ret &= AttachComponent(m_movement_component);

		ret &= m_skeletal_mesh_component->SetMesh(m_mesh_path);
		m_skeletal_mesh_component->SetLocalRotation(math::ToRadian(-90.0f), math::ToRadian(180.0f), 0.0f);
		m_skeletal_mesh_component->SetLocalScale(0.01f);
		//Notify 기능을 사용할 애니메이션을 미리 등록한다 
		//Notify 이름,애니메이션 이름, 특정 시간, 특정 시간에 실행할 함수
		//Notify 이름을 언리얼처럼 넣어주긴 했으나, 정작 사용하지는 않고 있다.
		m_skeletal_mesh_component->AddNotify("Attack End", "attack", 0.6f, 
			[this]() { m_is_attacking = false; /*LOG_INFO(m_is_attacking);*/ });
		m_skeletal_mesh_component->AddNotify("Hit End", "hit", 0.3f, 
			[this]() { m_is_hitting = false; /*LOG_INFO(m_is_attacking);*/ });

		ret &= AttachComponent(m_skeletal_mesh_component);
		m_player_fsm->Initialize(SharedFromThis());
		
		m_sphere_component->SetCollisionInfo(true, false, "default", { "default" }, true);
		ret &= AttachComponent(m_sphere_component);
		ret &= AttachComponent(m_camera_component);
	
		RegisterEvent();
		
		SetPosition(Vec3{ 2400.0f,300.0f,3400.0f });
		SetScale(0.5f);
		SetUseControllerPitch(false);
		SetUseControllerYaw(false);
		SetUseControllerRoll(false);

		return ret;
	}  

	void RevivePlayer::Shutdown()
	{
		m_movement_component = nullptr;
		m_skeletal_mesh_component = nullptr;
		m_sphere_component = nullptr;
		m_camera_component = nullptr;
		m_player_fsm->Shutdown();
		m_player_fsm = nullptr;
	}

	void RevivePlayer::Update(float delta_time)
	{
		Pawn::Update(delta_time);
		Vec3 current_position = GetPosition();
		if (current_position.y < 300)//땅의 높이
			current_position.y = 300;
		
		SetPosition(current_position);

		m_player_fsm->Update();
	}

	const float RevivePlayer::GetVelocity() const
	{
		return m_movement_component->GetVelocity().Length(); 
	}

	void RevivePlayer::SetAnimation(const std::string& animation_name, bool looping)
	{
		m_skeletal_mesh_component->SetAnimation(animation_name, looping);
		
	}

	void RevivePlayer::SetAnimationSpeed(float speed)
	{
		m_skeletal_mesh_component->SetAnimationSpeed(speed);
	}

	void RevivePlayer::SetMeshPosition(const Vec3& pos)
	{
		m_skeletal_mesh_component->SetLocalPosition(pos);
	}

	void RevivePlayer::RegisterEvent()
	{
		//테스트용 명령키
		//맞는 도중 다시 또 맞을 수 있다.
		RegisterPressedEvent("Hit", { {eKey::kP} },
			[this]()->bool { ++m_hit_count; m_is_hitting = true; DecrementHP();  return true; });

		//공격
		RegisterPressedEvent("attack", { {eKey::kLButton} },
			[this]()->bool { if (m_is_attacking == false) m_is_attacking = true; LOG_INFO(m_is_attacking);  return true; });

		//이동 및 조작
		RegisterAxisEvent("move forward", { AxisEventKeyInfo{eKey::kW, 1.0f}, AxisEventKeyInfo{eKey::kS, -1.0f} },
			[this](float axis)->bool { AddMovementInput(m_controller.lock()->GetForward(), axis); return true; });

		RegisterAxisEvent("move right", { AxisEventKeyInfo{eKey::kD, 1.0f}, AxisEventKeyInfo{eKey::kA, -1.0f} },
			[this](float axis)->bool { AddMovementInput(m_controller.lock()->GetRight(), axis); return true; });

		RegisterAxisEvent("turn", { AxisEventKeyInfo{eKey::kXMove, 1.0f} },
			[this](float axis)->bool {
			IVec2 relative_pos = Input::GetRelativeMousePosition();
			return AddControllerYawInput(axis * relative_pos.x);
		});

		RegisterAxisEvent("look up", { AxisEventKeyInfo{eKey::kYMove, 1.0f} },
			[this](float axis)->bool {
			IVec2 relative_pos = Input::GetRelativeMousePosition();
			MinPitch();
			return AddControllerPitchInput(axis * relative_pos.y);
		});
	}

	void RevivePlayer::AddMovementInput(Vec3& direction, float scale)
	{
		direction.y = 0;
		RotatePlayerFromCameraDirection(direction * scale);
		m_movement_component->AddInputVector(direction * scale);
	}

	void RevivePlayer::RotatePlayerFromCameraDirection(Vec3& dest_direction)
	{
		Vec3 old_player_forward = GetForward();

		if (vec3::Dot(old_player_forward, dest_direction) == -1)//현재보는 방향과 반대방향으로 회전 시 너무느리지않게 회전시키려면 필요한 부분
			old_player_forward += Vec3{ 0.02f,0.0f,-0.02f };

		float rotate_speed = 10.0f * 0.016f; //회전속도 delta_time을곱해야하나 없으니까 0.016곱함

		Vec3 curr_player_forward = old_player_forward + dest_direction * rotate_speed;
		curr_player_forward = vec3::Normalize(curr_player_forward);

		float angle = vec3::BetweenAngle(curr_player_forward, vec3::AXIS_Z); //0~PI만 반환함 (radian)

		if (vec3::Cross(curr_player_forward, vec3::AXIS_Z, true).y > 0.0f) //0~2PI값을 얻기위한 if문
			angle = -angle;

		auto player_rot = quat::CreateQuaternionFromRollPitchYaw(0.0f, angle, 0.0f); //angle만큼 Y축 회전
		SetRotation(player_rot);
	}

	void RevivePlayer::MinPitch()
	{
		Vec3 rot = quat::QuaternionToEuler(m_controller.lock()->GetRotation());
		if (math::ToDegrees(rot.x) < 5) //컨트롤러에서 SetRotation(rot);를 안해주는게 제일 좋은 방법
		{
			rot.x += math::ToRadian(5.0f) - rot.x;
			m_controller.lock()->SetRotation(quat::CreateQuaternionFromRollPitchYaw(rot.x, rot.y, rot.z));
		}
	}

	DefaultCharacter::DefaultCharacter(const std::string& name)
		:DefaultPawn(name)
	{
	}

	bool DefaultCharacter::Initialize()
	{
		bool ret = true;

		ret &= Pawn::Initialize();
		ret &= AttachComponent(m_movement_component);

		auto mesh = CreateSPtr<StaticMeshComponent>();
		ret &= mesh->SetMesh("../Contents/cube.obj");
		mesh->SetCollisionInfo(true, false, "default", { "default" },true);
		mesh->OnCollisionResponse([this](const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor,
			const SPtr<SceneComponent>& other_component) {
			CollisionResponse(component, other_actor, other_component);
		});
		ret &= AttachComponent(mesh);

		RegisterAxisEvent("move forward", { AxisEventKeyInfo{eKey::kW, 1.0f}, AxisEventKeyInfo{eKey::kS, -1.0f} },
			[this](float axis)->bool { AddMovementInput(GetForward(), axis); return true; });
		RegisterAxisEvent("move right", { AxisEventKeyInfo{eKey::kD, 1.0f}, AxisEventKeyInfo{eKey::kA, -1.0f} },
			[this](float axis)->bool { AddMovementInput(GetRight(), axis); return true; });
		RegisterAxisEvent("move up", { AxisEventKeyInfo{eKey::kE, 1.0f}, AxisEventKeyInfo{eKey::kQ, -1.0f} },
			[this](float axis)->bool { AddMovementInput(GetUp(), axis); return true; });
		RegisterAxisEvent("turn", { AxisEventKeyInfo{eKey::kXMove, 1.0f} },
			[this](float axis)->bool {
			IVec2 relative_pos = Input::GetRelativeMousePosition();
			return AddControllerYawInput(axis * relative_pos.x);
		});
		RegisterAxisEvent("look up", { AxisEventKeyInfo{eKey::kYMove, 1.0f} },
			[this](float axis)->bool {
			IVec2 relative_pos = Input::GetRelativeMousePosition();
			return AddControllerPitchInput(axis * relative_pos.y);
		});

		SetUseControllerPitch(true);
		SetUseControllerYaw(true);
		SetUseControllerRoll(true);
		return ret;
	}
	void DefaultCharacter::Shutdown()
	{
		m_movement_component = nullptr;
		Pawn::Shutdown();
	}
	void DefaultCharacter::Update(float delta_time)
	{
		Pawn::Update(delta_time);
	}
	void DefaultCharacter::CollisionResponse(const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor, const SPtr<SceneComponent>& other_component)
	{
		LOG_INFO("{0}가 {1}에 충돌했다", component->GetOwner().lock()->GetName(), other_component->GetName());
	}
	
}

