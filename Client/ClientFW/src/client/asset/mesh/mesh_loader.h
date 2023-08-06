#pragma once

namespace client_fw
{
	class Mesh;
	class StaticMesh;
	class Skeleton;
	class SkeletalMesh;
	class Material;
	class AnimationSequence;

	class MeshLoader
	{
	public:
		MeshLoader() = default;

		virtual SPtr<Mesh> LoadMesh(const std::string& path, const std::string& extension) const;

		virtual SPtr<StaticMesh> LoadObj(const std::string& path, const std::string& extension) const;
	};

	struct MeshData;
	struct BoneData; //데이터를 모두 모은뒤,바깥에서 한번에 Set하기위해 추가

	class RevLoader : public MeshLoader
	{
	public:

		RevLoader() = default;

		virtual SPtr<Mesh> LoadMesh(const std::string& path, const std::string& extension)  const override;
		virtual SPtr<SkeletalMesh> LoadRev(const std::string& path, const std::string& extension) const;
		
	private:
		UINT m_mesh_count = 0;
		bool is_single_mesh = true;
	private:
		void SaveRevData(SPtr<SkeletalMesh>& s_mesh, const UINT& lod, std::vector<MeshData>& mesh_data) const;

		//계층구조 형태의 파일(rev파일)을 읽음

		//true반환시 메시정보를 전부 읽었고 Animation 정보를 읽을 준비가 됬다는 것을 의미함
		bool LoadFrameHierArchy(std::ifstream& file, SPtr<Skeleton>& skeleton, std::vector<MeshData>& mesh_data, const std::string& path) const;

		void LoadMeshFromRevFile(std::ifstream& file, std::vector<MeshData>& mesh_data) const;
		void LoadSkinDeformations(std::ifstream& file, SPtr<BoneData>& bone_data) const;

		void InitializeMeshData(std::vector<MeshData>& mesh_data) const;
		//원래는 Frame이나 Frame == Bone 취급한다
		//fbx에서는 frame != Bone
	};
	
	namespace load_help
	{
		int ReadString(std::ifstream& file, std::string& str);
		int ReadString(std::ifstream& file, int length, std::string& str);
		void ReadUINT(std::ifstream& file, UINT& value);
		void ReadIntager(std::ifstream& file, int& value);
		void ReadFloat(std::ifstream& file, float& value);
		void ReadMat4(std::ifstream& file, Mat4& mat4); 
		void ReadVec2(std::ifstream& file, Vec2& vec2); 
		void ReadVec3(std::ifstream& file, Vec3& vec3);
		void ReadVec4(std::ifstream& file, Vec4& vec4);
		void ReadIVec4(std::ifstream& file, IVec4& ivec4);
	}

}


