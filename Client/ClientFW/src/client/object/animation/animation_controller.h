#pragma once
#include "client/asset/animation/animation_sequence.h"

namespace client_fw
{
	class SkeletalMeshComponent;
	class AnimationSeqnece;
	class Skeleton;

	struct BoneData;
	struct NotifyData
	{
		int frame_index;
		std::string animation_name;
		std::function<void()> notify_function;
	};

	class AnimationController : public std::enable_shared_from_this<AnimationController>
	{
	public:
		AnimationController();
		virtual ~AnimationController() = default;

		bool Initialize();
		void Update(float delta_time);

	private:
		
		void AnimToPlay(float delta_time);

		void CopyBoneTransformData();

	public:

		const bool IsLooping() { return m_looping; }

		const std::vector<Mat4>& GetBoneTransformData() { return m_bone_transform_resource; }
		
		const std::string GetAnimationPath(const std::string& animation_name);

		void SetAnimation(bool looping = true);

		void SetAnimationName(const std::string& animation_name); 
		const std::string GetAnimationName() { return m_animation_name; }

		void SetBoneData();

		const float GetCurretPlayTime() const { return m_time_pos; }

		void SetAnimationSpeed(float value) { m_animation_speed = value; }
		const float GetAnimationSpeed() const { return m_animation_speed; }
		void AddNotify(const std::string name, const std::string animation_name, int frame_index, const std::function<void()>& function);
		
		void SetOwner(const SPtr<SkeletalMeshComponent>& owner) { m_owner = owner; }
		SPtr<SkeletalMeshComponent> GetOwner() const { return m_owner.lock(); }

		const Mat4& FindTransformToSocketName(const std::string& socket_name);

	private:
		std::string m_animation_name;
		std::string m_ready_animation_name;

		bool m_is_update_animation = false;
		bool m_is_end_animation = false;
		bool m_looping = true;

		float m_start_time;
		float m_end_time;

		float m_time_pos = 0.0f;

		float m_animation_speed = 1.0f;

		int m_curr_frame = 0;

		UINT m_instance_index;
		SPtr<AnimationSequence> m_anim_seq = nullptr;
		SPtr<AnimationSequence> m_ready_anim_seq = nullptr;
		WPtr<SkeletalMeshComponent> m_owner;
		std::vector<WPtr<Skeleton>> m_cahce_skeleton;
		std::vector<Mat4> m_bone_offset;
		std::vector<Mat4> m_bone_transform_resource;//셰이더로 전달하기위한 데이터
		std::unordered_map<std::string, UINT> m_bone_socket_info; //최종 계산된 데이터를 뼈이름으로 찾기 위해 사용

		
		std::unordered_map<std::string, NotifyData> m_notify_map;
	};
}