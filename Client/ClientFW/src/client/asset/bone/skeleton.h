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

		//��Ʈ �� �������� ��ǥ
		//���� ���ӿ����� ������������̴����� �����Ѵ�
		Mat4 m_bone_transform = mat4::IDENTITY; 
		Mat4 m_bone_transform_T;
	public:
		Mat4 m_to_parent = mat4::IDENTITY; //�θ�������� �����ϴ� ���
		Vec3 m_scale; //��� ����ǰ�����?
		Vec3 m_translation;
		Vec3 m_rotation;

	private:
		void UpdateSkeletonTree(const Mat4& mat);

	public:
		void Update();

		const SPtr<Skeleton> FindBone(const std::string& m_bone_name);

		const Mat4& GetBoneTransform(); // m_bone_transform
		const Mat4& GetTranposeBoneTransform(); //�ʿ��� ������ Transpose�ؼ� ����

		const SPtr<Skeleton>& GetParent() { return m_parent; }
		const std::string& GetBoneName() { return m_bone_name; }

		void SetBoneName(const std::string& name) { m_bone_name = name; }
		void SetToParent(const Mat4& to_parent) { m_to_parent = to_parent; }
		void SetBoneTransform(const Mat4& transform) { m_bone_transform = transform; }
		void SetChild(SPtr<Skeleton>& bone);
		
	};
}

