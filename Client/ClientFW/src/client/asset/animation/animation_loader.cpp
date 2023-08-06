#include "stdafx.h"
#include "client/asset/animation/animation_loader.h"
#include "client/asset/animation/animation_sequence.h"
#include "client/asset/bone/skeleton.h"
#include "client/asset/core/asset_manager.h"
#include "client/asset/mesh/mesh_loader.h"


namespace client_fw
{
	using namespace load_help;
	SPtr<AnimationSequence> AnimationLoader::LoadAnimation(std::ifstream& file, const SPtr<Skeleton>& skeleton) const
	{
		SPtr<AnimationSequence> anim_seq = CreateSPtr<AnimationSequence>();
		std::string prefix;

		float start_time;
		float end_time;

		float weight;
		int animated_bone_count;
		std::vector<std::vector<SPtr<AnimationCurve>>> anim_curves;
		std::vector<SPtr<AnimationCurve>> curve;
		std::vector<SPtr<Skeleton>> cache_skeleton;

		int temp_int;

		ReadString(file, prefix); //"<AnimationSets>"
		ReadIntager(file, temp_int);//animation_sets_count : 0

		ReadString(file, prefix); //"<AnimationSets>"
		ReadIntager(file, temp_int); //animation_set_count : 1

		//AnimationSequence Load
		ReadString(file, prefix); //"<AnimationSets>"

		ReadFloat(file, start_time);
		ReadFloat(file, end_time);

		ReadString(file, prefix); //"<AnimationSets>"
		ReadIntager(file, temp_int); //animation_layers_count : 1
		ReadString(file, prefix); //"<AnimationSets>"
		ReadIntager(file, temp_int); //animation_layer_count : 0

		//AnimationTrack
		ReadIntager(file, animated_bone_count);
		ReadFloat(file, weight);

		SPtr<AnimationTrack> anim_track = CreateSPtr<AnimationTrack>();


		for (int i = 0; i < animated_bone_count; ++i)
		{
			ReadString(file, prefix); //"<AnimationSets>"
			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<AnimationCurve>:"):
				ReadIntager(file, temp_int); //curve_node_index == i

				ReadString(file, prefix); //"<AnimationSets>"
				auto& temp_skel = skeleton->FindBone(prefix);
				if (temp_skel)cache_skeleton.push_back(temp_skel);
				else
				{
					auto name_skel = prefix;
				}
				//AnimationCurve
				curve.resize(9);
				while (ReadString(file, prefix))
				{
					if (prefix.compare("</AnimationCurve>") == 0) break;

					switch (HashCode(prefix.c_str()))
					{ //0 : TX 1 : TY ... 8 : SZ 번호에 맞춰서
					case HashCode("<TX>:"):
						curve[0] = LoadKeyValue(file);
						break;
					case HashCode("<TY>:"):
						curve[1] = LoadKeyValue(file);
						break;
					case HashCode("<TZ>:"):
						curve[2] = LoadKeyValue(file);
						break;
					case HashCode("<RX>:"):
						curve[3] = LoadKeyValue(file);
						break;
					case HashCode("<RY>:"):
						curve[4] = LoadKeyValue(file);
						break;
					case HashCode("<RZ>:"):
						curve[5] = LoadKeyValue(file);
						break;
					case HashCode("<SX>:"):
						curve[6] = LoadKeyValue(file);
						break;
					case HashCode("<SY>:"):
						curve[7] = LoadKeyValue(file);
						break;
					case HashCode("<SZ>:"):
						curve[8] = LoadKeyValue(file);
						break;

					}
				}

				anim_curves.emplace_back(std::move(curve));
				//end
				break;
			}
		}
		anim_track->InitialIze(static_cast<int>(cache_skeleton.size()), weight);
		anim_track->SetAnimationCurves(anim_curves);
		anim_track->SetCacheSkel(cache_skeleton);
		//end
		anim_seq->SetAnimationTrack(anim_track);
		anim_seq->SetDefaultTime(start_time, end_time);
		ReadString(file, prefix); //"</AnimationLayer>"
		ReadString(file, prefix); //"</AnimationLayers>"
		ReadString(file, prefix); //"<AnimationSet>"
		ReadString(file, prefix); //"<AnimationSets>"

		return anim_seq;
	}

	std::ifstream AnimationLoader::GetFilePointerForAnimation(const std::string& path, const std::string& extension) const
	{
		std::ifstream file(path, std::ios::binary);

		if (file.is_open() == false)
		{
			LOG_ERROR("Could not find path : [{0}]", path);
			return file;
		}

		//정보를 모으는 벡터들은 재귀함수 바깥쪽에 있어야 한다
		std::string prefix;
		while (ReadString(file, prefix))
		{
			if (prefix == "</Animation>") break;

			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<Hierarchy>"):

				while (ReadString(file, prefix))
				{
					if (prefix == "<Frame>:")
					{
						ReadFrameHierArchy(file);
					}
					else if (prefix == "</Hierarchy>")
						break;
				}
				break;

			case HashCode("<Animation>"):
				return file;
			default:
				break;
			}
		}
		return file;
	}

	void AnimationLoader::ReadFrameHierArchy(std::ifstream& file) const
	{
		std::string prefix;

		std::string temp_string;
		UINT temp_uint;
		Mat4 temp_mat4;
		Vec3 temp_vec3;

		ReadUINT(file, temp_uint);
		ReadString(file, temp_string); //bone name read

		while (ReadString(file, prefix))
		{
			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<Transform>:"):
				ReadMat4(file, temp_mat4);
				for (UINT i = 0; i < 3; ++i)
					ReadVec3(file, temp_vec3);
				break;
			case HashCode("<Mesh>:"):
				ReadMesh(file);
				break;
			case HashCode("<SkinDeformations>:"):
				ReadSkinDeformations(file);
				ReadString(file, prefix);
				if (prefix == ("<Mesh>:"))
					ReadMesh(file);
				break;
			case HashCode("<Materials>:"):
				ReadUINT(file, temp_uint);
				while (ReadString(file, prefix))
				{
					if (prefix == ("</Materials>")) break;
					switch (HashCode(prefix.c_str()))
					{
					case HashCode("<Material>:"):
						ReadUINT(file, temp_uint);
						break;
					case HashCode("<AlbedoMap>:"):
						ReadString(file, prefix); //W_HEAD_00_violet +확장자 붙힌채로 읽기
						break;
					}
				}
				break;
			case HashCode("<Children>:"):
				ReadUINT(file, temp_uint);
				for (UINT i = 0; i < temp_uint; ++i)
				{
					ReadString(file, prefix);
					if (prefix == ("<Frame>:"))
						ReadFrameHierArchy(file);
				}
				break;
			case HashCode("</Frame>"):
				return;
			}
		}
	}
	void AnimationLoader::ReadMesh(std::ifstream& file) const
	{
		std::string prefix;

		Vec3 temp_vec3;
		Vec2 temp_vec2;
		UINT temp_uint;

		ReadString(file, prefix);

		while (ReadString(file, prefix))
		{
			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<Bounds>:"):
				ReadVec3(file, temp_vec3);
				ReadVec3(file, temp_vec3);
				break;
			case HashCode("<ControlPoints>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
				break;

			case HashCode("<TextureCoords0>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec2(file, temp_vec2);

				break;

			case HashCode("<TextureCoords1>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec2(file, temp_vec2);
				break;

			case HashCode("<Normals>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
				break;

			case HashCode("<Tangents>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
				break;

			case HashCode("<BiTangents>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
				break;

			case HashCode("<Polygons>:"):
				ReadUINT(file, temp_uint);
				while (ReadString(file, prefix))
				{
					if (prefix.compare("<SubIndices>:") == 0)
					{
						ReadUINT(file, temp_uint);
						ReadUINT(file, temp_uint);
						while (ReadString(file, prefix))
						{

							if (prefix == ("</Polygons>")) break;
							else if (prefix == ("<SubIndex>:"))
							{
								ReadUINT(file, temp_uint);
								ReadUINT(file, temp_uint);
								UINT count = temp_uint;
								if (count > 0)
									for (UINT i = 0; i < count; i++)
										ReadUINT(file, temp_uint);
							}
						}

					}
					if (prefix == ("</Polygons>")) break;

				}
				break;

			case HashCode("</Mesh>"):
				return;
			}
		}
	}
	void AnimationLoader::ReadSkinDeformations(std::ifstream& file) const
	{
		std::string prefix;

		Mat4 temp_mat4;
		Vec3 temp_vec3;
		Vec4 temp_vec4;
		IVec4 temp_ivec4;
		UINT temp_uint;

		while (ReadString(file, prefix))
		{

			if (prefix == ("</SkinDeformations>")) break;

			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<BonesPerVertex>:"):
				ReadUINT(file, temp_uint);
				break;
			case HashCode("<Bounds>:"):
				ReadVec3(file, temp_vec3);
				ReadVec3(file, temp_vec3);
				break;
			case HashCode("<BoneNames>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadString(file, prefix);
				break;
			case HashCode("<BoneOffsets>:"):
				ReadUINT(file, temp_uint);

				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadMat4(file, temp_mat4);
				break;
			case HashCode("<BoneIndices>:"):
				ReadUINT(file, temp_uint);

				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadIVec4(file, temp_ivec4);
				break;
			case HashCode("<BoneWeights>:"):
				ReadUINT(file, temp_uint);
				if (temp_uint > 0)
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec4(file, temp_vec4);
				break;
			default:
				break;
			}
		}

	}

	SPtr<AnimationCurve> AnimationLoader::LoadKeyValue(std::ifstream& file) const
	{
		SPtr<AnimationCurve> anim_curve = CreateSPtr<AnimationCurve>();

		int temp_int;

		ReadIntager(file,temp_int); //key_frame_count
		std::vector<KeyFrame> key_frames(temp_int);


		for (auto& key_frame : key_frames)
			ReadFloat(file, key_frame.key_time);
		for (auto& key_frame : key_frames)
			ReadFloat(file, key_frame.key_value);

		anim_curve->m_key_frames = key_frames;

		return anim_curve;
	}

}