#pragma once
#include <client/object/actor/pawn.h>
#include <client/object/actor/default_pawn.h>

enum class COLOR_TYPE;

namespace client_fw
{
	class SceneComponent;
	class SkeletalMeshComponent;
	class BoxComponent;
	class SphereComponent;
	class RenderCameraComponent;
	class SpringArmRenderCameraComponent;
	class SimpleMovementComponent;
	class CharacterMovementComponent;
	class SpotLightComponent;
}

namespace revive
{
	using namespace client_fw;

	class Pistol;
	class PlayerFSM;

	class DefaultPlayer : public Pawn
	{
	public:
		DefaultPlayer(const std::string& name = "player");
		virtual ~DefaultPlayer() = default;

		virtual bool Initialize() override;
		virtual void Shutdown() override;
		virtual void Update(float delta_time) override;
		virtual void ExecuteMessageFromServer(const SPtr<MessageEventInfo>& message) override;

	protected:
		std::string m_player_name = "revive";
		float m_hp = 1;
		float m_max_hp = 1;
		int m_hit_count = 0;//맞는 도중 또 맞는 경우를 위해 만듬
		int m_network_id = 9999;
		float m_time = 0.0f;
		float m_stop_time = 0.0f;
		float m_speed = 0.f;

		bool m_is_attacking = false;
		bool m_is_hitting = false;
		bool m_is_dying = false;
		bool m_is_fire = false;
		bool m_is_equipped = false;

		Vec3 m_velocity;
		Vec3 m_previous_velocity = vec3::ZERO;
		Vec3 m_inter_velocity;
		Vec3 m_next_pos;
		Vec3 m_previous_pos;
		Vec3 m_attack_direction;

		std::string m_mesh_path;
		SPtr<PlayerFSM> m_player_fsm;
		SPtr<CharacterMovementComponent> m_character_movement_component;
		SPtr<SkeletalMeshComponent> m_skeletal_mesh_component;
		SPtr<BoxComponent> m_hit_box;
		std::array<SPtr<Pistol>, 2> m_weapon;

		virtual Quaternion FindLookAtRotation(const Vec3& start, const Vec3& target);
		void PlayerInterpolation(float delta_time);

		SPtr<DefaultPlayer> SharedFromThis() { return std::static_pointer_cast<DefaultPlayer>(shared_from_this()); }

	public:
		virtual const float GetVelocity() const;
		const int GetHitCount() const { return m_hit_count; }
		const std::string& GetPlayerName() const { return m_player_name; }
		void SetPlayerName(const std::string& name) { m_player_name = name; }
		const float GetHP() const { return m_hp; }
		void SetHP(float hp);
		const float GetMaxHP() const { return m_max_hp; }
		void SetMaxHP(float max_hp) { m_max_hp = max_hp; }
		void GetAnimationSpeed() const;
		void SetAnimationSpeed(float speed);

		const bool IsAttacking() const { return m_is_attacking; }
		const bool IsHitting() const { return m_is_hitting; }
		const bool IsDying() const { return m_is_dying; }
		void SetIsDying(bool value) { m_is_dying = value; }

		bool SetColor(COLOR_TYPE color_type);
		void SetMeshPosition(const Vec3& pos);
		void SetAnimation(const std::string& animation_name, bool looping = true);
		void DecrementHitCount() { m_hit_count--; }

		const int GetNetworkID()const { return m_network_id; }
		void SetNetworkID(int val) { m_network_id = val; };
		void SetNetworkPosition(const Vec3& pos);


		virtual void Attack();
		virtual void Hit(float damage = 1, int nw_id = 9999);
		const bool IsEquipped() const { return m_is_equipped; }
		virtual void Equip();
		virtual void Unequip();

	protected:
		SPtr<SpotLightComponent> m_spot_light_component;

		std::function<void(float, float)> m_changed_hp_function;

	public:
		void OnChangedHPFunction(std::function<void(float, float)>&& function) { m_changed_hp_function = function; }

	};

	class RevivePlayer : public DefaultPlayer
	{
	public:
		RevivePlayer(bool is_develop_mode, const std::string& name = "Player");
		virtual ~RevivePlayer() = default;

		virtual bool Initialize() override;
		virtual void Shutdown() override;
		virtual void Update(float delta_time) override;
		virtual void ExecuteMessageFromServer(const SPtr<MessageEventInfo>& message) override;

	private:
		bool m_is_develop_mode = false;
		bool m_is_hp_cheating = false;
		bool m_is_damage_cheating = false;

		void AddMovementInput(const Vec3& direction, float scale) override;
		
		void RegisterEvent(); //등록할 것이 많아져서 따로 분리했다.

		//player의 state를 관리하는 객체
		//원래는 State자체를 저장하고 플레이어 Update에서 바꿔주려고 했으나,
		//이렇게 할 경우 캐스팅이 매번 일어나기 때문에 PlayerFSM에서 관리만 해주는 형태로 변경함

		SPtr<CharacterMovementComponent> m_movement_component;
		SPtr<SphereComponent> m_blocking_sphere;
		SPtr<SpringArmRenderCameraComponent> m_camera_component;

		using Pawn::SetUseControllerPitch;
		using Pawn::SetUseControllerYaw;
		using Pawn::SetUseControllerRoll;

		SPtr<RevivePlayer> SharedFromThis() { return std::static_pointer_cast<RevivePlayer>(shared_from_this()); }

	public:
		void CollisionResponse(const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor, const SPtr<SceneComponent>& other_component) {};

		//void SetIsAttacking(bool value) { m_is_attacking = value; }
		//void SetIsHitting(bool value) { m_is_hitting = value; }
		const float GetVelocity() const;
		const int GetHitCount() const { return m_hit_count; }


		virtual void Attack() override;

		virtual void Hit(float damage, int nw_id = 9999) override;


		void DecrementHP() { m_hp--; LOG_INFO("my HP : {0}", m_hp); }

		//void RotateFromDirection(const Vec3& direction);
		//Controller에서 호출
		//void MinPitch(); //최소 Pitch 제한을 걸기 위한 함수
		void FixYPosition();

	};

	class DefaultCharacter : public DefaultPawn
	{
	public:
		DefaultCharacter(const std::string& name = "default character");
		virtual ~DefaultCharacter() = default;

		virtual bool Initialize() override;
		virtual void Shutdown() override;
		virtual void Update(float delta_time) override;

		void CollisionResponse(const SPtr<SceneComponent>& component, const SPtr<Actor>& other_actor, const SPtr<SceneComponent>& other_component);

	private:
		using Pawn::SetUseControllerPitch;
		using Pawn::SetUseControllerYaw;
		using Pawn::SetUseControllerRoll;

	};
}