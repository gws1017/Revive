#pragma once
namespace client_fw
{
	class Skeleton : public std::enable_shared_from_this<Skeleton>
	{
	public:
		Skeleton() = default;
		virtual ~Skeleton() = default;

	private:
		SPtr<Skeleton> m_parent = nullptr;
		SPtr<Skeleton> m_child = nullptr;
		SPtr<Skeleton> m_sibling = nullptr;

		std::string m_bone_name;

		//루트 뼈 공간기준 transform
		//실제 게임에서의 월드공간은셰이더에서 적용한다
		Mat4 m_root_transform = mat4::IDENTITY; 
		Mat4 m_root_transform_T;
	public:
		Mat4 m_transform = mat4::IDENTITY; //현재 뼈 공간에서의 transform

		Mat4 m_scale_matrix;
		Mat4 m_translation_matrix;
		Mat4 m_rotation_matrix;

		Vec3 m_scale; 
		Vec3 m_translation;
		Vec3 m_rotation;

	private:
		void UpdateSkeletonTree(const Mat4& mat);

		const Mat4& ToRootBone(const Mat4& child_vec);
		const Vec3& Skeleton::ToRootBone(const Vec3& child_vec);

		void SetRootTransform(const Mat4& transform) { m_root_transform = transform; }

	public:
		void Update();

		const SPtr<Skeleton> FindBone(const std::string& m_bone_name);

		const Mat4& GetRootTransform() { return m_root_transform; };
		const Mat4& GetTranposeRootTransform(); //필요할 때마다 Transpose해서 전달

		const Vec3& GetBoneScale() { return ToRootBone(m_scale); }
		const Vec3& GetBoneTranslation() { return ToRootBone(m_translation); }
		const Vec3& GetBoneRotation() { return ToRootBone(m_rotation); }

		const Mat4& GetBoneScaleMatrix() { return ToRootBone(m_scale_matrix); }
		const Mat4& GetBoneTranslationMatrix() { return ToRootBone(m_translation_matrix); }
		const Mat4& GetBoneRotationMatrix() { return ToRootBone(m_rotation_matrix); }

		const SPtr<Skeleton>& GetParent() { return m_parent; }
		const std::string& GetBoneName() { return m_bone_name; }

		void SetBoneName(const std::string& name) { m_bone_name = name; }
		void SetTransform(const Mat4& transform) { m_transform = transform; }
		void SetChild(SPtr<Skeleton>& bone);
		
	};
}

