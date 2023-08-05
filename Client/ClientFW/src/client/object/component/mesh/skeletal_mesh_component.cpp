#include "stdafx.h"
#include "client/asset/animation/animation_sequence.h"
#include "client/object/component/mesh/skeletal_mesh_component.h"
#include "client/object/actor/core/actor.h"
#include "client/asset/core/asset_store.h"
#include "client/asset/mesh/mesh.h"

namespace client_fw
{
	SkeletalMeshComponent::SkeletalMeshComponent(const std::string& name, const std::string& draw_shader_name)
		:MeshComponent(name, draw_shader_name)
	{
		m_animation_controller = CreateSPtr<AnimationController>();
	}

	bool SkeletalMeshComponent::Initialize()
	{
		m_animation_controller->Initialize();
		return MeshComponent::Initialize();
	}

	void SkeletalMeshComponent::Update(float delta_time)
	{
		if (m_is_playing)
		{
			GetSkeletalMesh()->GetSkeleton()->Update();
			m_animation_controller->Update(delta_time);
		}
	}

	void SkeletalMeshComponent::Shutdown()
	{
		MeshComponent::Shutdown();
	}
	
	SPtr<SkeletalMesh> SkeletalMeshComponent::GetSkeletalMesh() const
	{
		return std::static_pointer_cast<SkeletalMesh>(m_mesh);
	}

	bool SkeletalMeshComponent::SetMesh(const std::string& file_path)
	{
		m_mesh = std::dynamic_pointer_cast<SkeletalMesh>(AssetStore::LoadMesh(file_path));
		if (m_mesh == nullptr)
		{
			LOG_ERROR("Could not cast Mesh[{0}] to SkeletalMesh", file_path);
			return m_set_mesh;
		}
		else m_set_mesh = true;

		m_animation_controller->SetOwner(SharedFromThis());
		m_animation_controller->SetBoneData();
		
		return m_set_mesh;
	}
	void SkeletalMeshComponent::SetAnimation(const std::string& animation_name,bool looping)
	{
		if (m_set_mesh == true)
		{
			m_animation_name = animation_name;
			
			if (animation_name == "Null")
				SetIsPlaying(false);
			else 
			{
				SetIsPlaying(true);
				m_animation_controller->SetAnimationName(animation_name);
				m_animation_controller->SetAnimation(looping);
			}
		}
	}

	const Vec3 SkeletalMeshComponent::GetBoneWorldPosition(const std::string& bone_name)
	{
		const auto& world_matrix = GetBoneWorldMatrix(bone_name);
		return Vec3{ world_matrix._41,world_matrix._42,world_matrix._43};
	}

	const Mat4 SkeletalMeshComponent::GetBoneWorldMatrix(const std::string& bone_name)
	{
		Mat4 bone_matrix = m_animation_controller->FindTransformToBoneName(bone_name);
		bone_matrix.Transpose();
		return bone_matrix * GetWorldMatrix();
	}

	const Quaternion SkeletalMeshComponent::GetBoneWorldRotation(const std::string& bone_name)
	{
		Mat4 bone_matrix = m_animation_controller->FindTransformToBoneName(bone_name);
		bone_matrix.Transpose();
		Quaternion bone_rotation;
		XMStoreFloat4(&bone_rotation, XMQuaternionRotationMatrix(XMLoadFloat4x4(&bone_matrix)));

		return bone_rotation * GetWorldRotation();
	}

	SPtr<SkeletalMeshComponent> SkeletalMeshComponent::SharedFromThis()
	{
		return std::static_pointer_cast<SkeletalMeshComponent>(shared_from_this());
	}
}