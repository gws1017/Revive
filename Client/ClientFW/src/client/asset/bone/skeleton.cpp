#include "stdafx.h"
#include "skeleton.h"

namespace client_fw 
{
	const Mat4& Skeleton::GetBoneTransform()
	{
		return m_bone_transform;
	}
	const Mat4& Skeleton::GetTranposeBoneTransform()
	{
		m_bone_transform_T = m_bone_transform;
		m_bone_transform_T.Transpose();
		return m_bone_transform_T;
	}
	const Mat4& Skeleton::ToRootBone(const Mat4& child_mat)
	{
		if (m_parent == nullptr)
			return child_mat;
		else 
		{
			m_parent->ToRootBone(m_to_parent * child_mat);
		}
	}

	const Vec3& Skeleton::ToRootBone(const Vec3& child_vec)
	{
		if (m_parent == nullptr)
			return child_vec;
		else
		{
			m_parent->ToRootBone(vec3::TransformCoord(child_vec, m_to_parent));
		}
	}

	void Skeleton::UpdateSkeletonTree(const Mat4& parent_transform)
	{
		m_bone_transform = m_to_parent * parent_transform;
		
		if (m_sibling)
			m_sibling->UpdateSkeletonTree(parent_transform);
		if (m_child)
			m_child->UpdateSkeletonTree(m_bone_transform);
	}

	void Skeleton::Update()
	{
		UpdateSkeletonTree(mat4::IDENTITY);
	}
		
	void Skeleton::SetChild(SPtr<Skeleton>& bone)
	{
		if (bone == nullptr) return;

		bone->m_parent = shared_from_this();

		if (m_child == nullptr)
			m_child = bone;
		else
		{
			bone->m_sibling = m_child->m_sibling;
			m_child->m_sibling = bone;
		}
	}

	const SPtr<Skeleton> Skeleton::FindBone(const std::string& bone_name)
	{
		if (m_bone_name == bone_name)
			return shared_from_this();

		SPtr<Skeleton> cache_skeleton = nullptr;
		if (m_sibling)
		{
			if (cache_skeleton = m_sibling->FindBone(bone_name))
				return cache_skeleton;
		}
		if (m_child)
		{
			if(cache_skeleton = m_child->FindBone(bone_name))
				return cache_skeleton;
		}

		return nullptr;
	}

}
