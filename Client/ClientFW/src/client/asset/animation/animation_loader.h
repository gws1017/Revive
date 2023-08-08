#pragma once

namespace client_fw
{
	class Skeleton;
	class AnimationCurve;
	class AnimationSequence;
	
	class AnimationLoader
	{
	public:
		AnimationLoader() = default;

		virtual SPtr<AnimationSequence> LoadAnimation(std::ifstream& file, const SPtr<Skeleton>& skeleton) const;
		std::ifstream GetFilePointerForAnimation(const std::string& path, const std::string& extension) const;

	private:

		void ReadFrameHierArchy(std::ifstream& file) const; //LoadFrameHierArchy()�Լ��� �����ϳ� ������ �����ϴºκ��� ���ݴ�.
		void ReadMesh(std::ifstream& file) const;
		void ReadSkinDeformations(std::ifstream& file) const;
		SPtr<AnimationCurve> LoadKeyValue(std::ifstream& file) const;

	};
}
