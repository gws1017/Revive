#include "stdafx.h"
#include "client/asset/mesh/mesh_loader.h"
#include "client/asset/mesh/mesh.h"
#include "client/asset/primitive/vertex.h"
#include "client/asset/bone/skeleton.h"
#include "client/asset/animation/animation_sequence.h"
#include "client/asset/core/asset_manager.h"
#include "client/asset/core/asset_store.h"
#include "client/asset/material/material.h"
#include "client/physics/core/bounding_mesh.h"
#include "client/physics/collision/mesh_bounding_tree.h"

namespace client_fw
{
	SPtr<Mesh> MeshLoader::LoadMesh(const std::string& path, const std::string& extension) const
	{
		SPtr<Mesh> mesh = nullptr;

		switch (HashCode(extension.c_str()))
		{
		case HashCode(".obj"):
			mesh = LoadObj(path, extension);
			break;
		default:
			LOG_ERROR("Files in {0} format cannot be supported", extension);
			break;
		}

		return mesh;
	}

	struct CombineData
	{
		std::string mtl_name;
		std::vector<UINT> pos_indices;
		std::vector<UINT> tex_indices;
		std::vector<UINT> normal_indices;
	};

	SPtr<StaticMesh> MeshLoader::LoadObj(const std::string& path, const std::string& extension) const
	{
		std::ifstream obj_file(path);
		
		if (obj_file.is_open() == false)
		{
			LOG_ERROR("Could not find path : [{0}]", path);
			return nullptr;
		}

		std::string parent_path = file_help::GetParentPathFromPath(path);
		std::string stem = file_help::GetStemFromPath(path);

		UINT lod = 0;
		SPtr<StaticMesh> mesh = CreateSPtr<StaticMesh>();

		while (lod < 4)
		{
			std::vector<Vec3> positions;
			std::vector<Vec3> normals;
			std::vector<Vec2> tex_coords;
			std::map<std::string, SPtr<Material>> materials;
			std::vector<CombineData> combine_data;
			UINT combine_data_index = 0;

			std::stringstream ss;
			std::string line;
			std::string prefix;

			Vec3 temp_vec;
			UINT temp_uint = 0;
			std::string temp_string;

			while (std::getline(obj_file, line))
			{
				ss.clear();
				prefix.clear();
				ss.str(line);
				ss >> prefix;

				switch (HashCode(prefix.c_str()))
				{
				case HashCode("mtllib"):
				{
					ss >> temp_string;
					materials = AssetStore::LoadMaterials(file_help::GetParentPathFromPath(path) + "\\" + temp_string);
					break;
				}
				case HashCode("v"):
					ss >> temp_vec.x >> temp_vec.y >> temp_vec.z;
					temp_vec.z *= -1.0f;
					positions.emplace_back(std::move(temp_vec));
					break;
				case HashCode("vt"):
					ss >> temp_vec.x >> temp_vec.y >> temp_vec.z;
					temp_vec.y = 1.0f - temp_vec.y;
					tex_coords.emplace_back(Vec2{ temp_vec.x, temp_vec.y });
					break;
				case HashCode("vn"):
					ss >> temp_vec.x >> temp_vec.y >> temp_vec.z;
					temp_vec.z *= -1.0f;
					normals.emplace_back(std::move(temp_vec));
					break;
				case HashCode("usemtl"):
				{
					ss >> temp_string;
					auto iter = std::find_if(combine_data.cbegin(), combine_data.cend(),
						[temp_string](const CombineData& data) {
							return data.mtl_name == temp_string;
						});
					if (iter == combine_data.cend())
					{
						combine_data.push_back({ temp_string });
						combine_data_index = static_cast<UINT>(combine_data.size()) - 1;
					}
					else
					{
						combine_data_index = static_cast<UINT>(std::distance(combine_data.cbegin(), iter));
					}
				}
				break;
				case HashCode("f"):
				{
					int count = 0;

					while (ss >> temp_uint)
					{
						if (count == 0)
							combine_data[combine_data_index].pos_indices.emplace_back(temp_uint - 1);
						else if (count == 1)
							combine_data[combine_data_index].tex_indices.emplace_back(temp_uint - 1);
						else if (count == 2)
							combine_data[combine_data_index].normal_indices.emplace_back(temp_uint - 1);

						if (ss.peek() == '/')
						{
							++count;
							ss.ignore();
						}
						else if (ss.peek() == ' ')
						{
							ss.ignore(1, ' ');
							count = 0;
						}
					}
				}
				break;
				default:
					break;
				}
			}

			mesh->CreateDataForLodMesh(lod);

			UINT vertex_count = 0;
			for (const auto& data : combine_data)
				vertex_count += static_cast<UINT>(data.pos_indices.size());

			std::vector<TextureLightNormalMapVertex> vertices;
			vertices.reserve(vertex_count);
			std::vector<Triangle> triangles;
			triangles.reserve(vertex_count / 3);

			vertex_count = 0;
			for (const auto& data : combine_data)
			{
				UINT count = 0;
				for (size_t i = 0; i < data.pos_indices.size() / 3; ++i)
				{
					size_t index = i * 3;

					Vec3 v1 = positions[data.pos_indices[index + 2]];
					Vec3 v2 = positions[data.pos_indices[index + 1]];
					Vec3 v3 = positions[data.pos_indices[index]];

					Vec3 normal = vec3::Cross(v3 - v1, v2 - v1, true);
					if (normal == vec3::ZERO) continue;

					triangles.emplace_back(Triangle{ v1, v2, v3, normal });

					Vec2 uv1 = tex_coords[data.tex_indices[index + 2]];
					Vec2 uv2 = tex_coords[data.tex_indices[index + 1]];
					Vec2 uv3 = tex_coords[data.tex_indices[index]];

					Vec3 delta_pos1 = v2 - v1;
					Vec3 delta_pos2 = v3 - v1;

					Vec2 delta_uv1 = uv2 - uv1;
					Vec2 delta_uv2 = uv3 - uv1;

					float f = 1.0f / (delta_uv1.x * delta_uv2.y - delta_uv2.x * delta_uv1.y);

					Vec3 tangent;
					tangent.x = f * (delta_uv2.y * delta_pos1.x - delta_uv1.y * delta_pos2.x);
					tangent.y = f * (delta_uv2.y * delta_pos1.y - delta_uv1.y * delta_pos2.y);
					tangent.z = f * (delta_uv2.y * delta_pos1.z - delta_uv1.y * delta_pos2.z);

					Vec3 bitangent;
					bitangent.x = f * (-delta_uv2.x * delta_pos1.x + delta_uv1.x * delta_pos2.x);
					bitangent.y = f * (-delta_uv2.x * delta_pos1.y + delta_uv1.x * delta_pos2.y);
					bitangent.z = f * (-delta_uv2.x * delta_pos1.z + delta_uv1.x * delta_pos2.z);

					for (INT j = 2; j >= 0; --j)
					{
						TextureLightNormalMapVertex vertex;
						vertex.SetPosition(positions[data.pos_indices[index + j]]);
						vertex.SetTexCoord(tex_coords[data.tex_indices[index + j]]);
						vertex.SetNormal(normals[data.normal_indices[index + j]]);
						vertex.SetTangent(tangent);
						vertex.SetBitangent(bitangent);
						vertices.emplace_back(std::move(vertex));
					}			

					count += 3;
				}

				mesh->AddMeshVertexInfo(lod, { count, vertex_count });
				mesh->AddMaterial(lod, std::move(materials[data.mtl_name]));

				vertex_count += count;
			}

			const auto& vertex_info = mesh->GetVertexInfo(lod);
			if(vertex_info->CreateVertexBlob<TextureLightNormalMapVertex>(vertex_count)==false)
			{
				LOG_ERROR("Could not create blob for vertex");
				return nullptr;
			}
			vertex_info->CopyData(vertices.data(), vertex_count);

			if (lod == 0)
			{
#ifdef SHOW_TREE_INFO
				LOG_INFO("Triangle Tree : {0}", path);
#endif // SHOW_TREE_INFO
				BOrientedBox box = BOrientedBox(std::move(positions));
				mesh->SetOrientBox(box);
				/*auto bounding_tree = CreateSPtr<KDTree>();
				bounding_tree->Initialize(box, triangles);
				mesh->SetBoundingTree(std::move(bounding_tree));*/
			}

			++lod;
			std::string lod_path = parent_path + "/" + stem + "_lod" + std::to_string(lod) + extension;
			obj_file = std::ifstream(lod_path);

			if (obj_file.is_open() == false)
				break;
		}

		return mesh;
	}

	using namespace load_help;

	struct MeshData
	{
		std::vector<Vec3>positions;
		std::vector<Vec2>tex_coords;
		std::vector<Vec3>normals;
		std::vector<UINT> incdices;
		std::vector<std::string> mtl_names;
		std::map<std::string, SPtr<Material>> materials;
		SPtr<BoneData> bone_data;
		std::vector<BOrientedBox> oriented_boxes;
	};

	SPtr<Mesh> RevLoader::LoadMesh(const std::string& path, const std::string& extension) const
	{
		SPtr<Mesh> mesh = nullptr;
		static_cast<UINT>(m_mesh_count) = 0;
		switch (HashCode(extension.c_str()))
		{
		case HashCode(".obj"):
			mesh = LoadObj(path, extension);
			break;
		case HashCode(".rev"):
			mesh = LoadRev(path, extension);
			break;
		default:
			LOG_ERROR("Files in {0} format cannot be supported", extension);
			break;
		}

		return mesh;
	}

	SPtr<SkeletalMesh> RevLoader::LoadRev(const std::string& path, const std::string& extension) const
	{
		std::ifstream rev_file(path,std::ios::binary);

		if (rev_file.is_open() == false)
		{
			LOG_ERROR("Could not find path : [{0}]", path);
			return nullptr;
		}

		std::string parent_path = file_help::GetParentPathFromPath(path);
		std::string stem = file_help::GetStemFromPath(path);

		UINT lod = 0;

		SPtr<SkeletalMesh> s_mesh = CreateSPtr<SkeletalMesh>();
		SPtr<Skeleton> skeleton = CreateSPtr<Skeleton>();
		skeleton->SetBoneName("Root");
		
		std::vector<MeshData> mesh_data;
		InitializeMeshData(mesh_data);

		std::string prefix;
		while (ReadString(rev_file, prefix))
		{
			if (prefix == "</Animation>") break;

			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<Hierarchy>"):

				while (ReadString(rev_file, prefix))
				{
					if (prefix == "<Frame>:")
					{
						SPtr<Skeleton> child = CreateSPtr<Skeleton>();
						skeleton->SetChild(child);
						LoadFrameHierArchy(rev_file, child, mesh_data, path);
					}
					else if (prefix == "</Hierarchy>")
					{
						SaveRevData(s_mesh, lod, mesh_data);
						s_mesh->SetSkeleton(skeleton);
		
						break;
					}
				}
				break;

			case HashCode("<Animation>"):
			{
				return s_mesh;
			}
			default:
				break;
			}
		}

		return s_mesh;
	}

	
	void RevLoader::SaveRevData(SPtr<SkeletalMesh>& s_mesh, const UINT& lod, std::vector<MeshData>& mesh_data) const
	{
		s_mesh->CreateDataForLodMesh(lod);
		s_mesh->SetBoneData(mesh_data.at(0).bone_data);

		std::vector<Vec3> positions;
		UINT mesh_index = (m_mesh_count == 0) ? 0 : m_mesh_count - 1;

		UINT vertex_count = 0;
		UINT index_count = 0;

		for (auto& data : mesh_data)
		{
			for (auto& name : data.mtl_names)
			{
				auto temp_material = data.materials[name];
				if (temp_material)s_mesh->AddMaterial(lod, std::move(temp_material));
			}

			vertex_count += static_cast<UINT>(data.positions.size());
			index_count += static_cast<UINT>(data.incdices.size());
		}

		std::vector<BoneVertex> vertices(vertex_count);
		std::vector<UINT> indices(index_count);

		vertex_count = 0;
		index_count = 0;

		for (auto& data : mesh_data) //메시가 여러개인데 합칠거임
		{

			UINT v_count = static_cast<UINT>(data.positions.size());
			for (UINT i = 0; i < v_count; ++i)
			{
				UINT index = i + vertex_count;
				vertices[index].SetPosition(data.positions[i]);
				vertices[index].SetTexCoord(data.tex_coords[i]);
				vertices[index].SetNormal(data.normals[i]);
				vertices[index].SetBoneWeight(data.bone_data->bone_weights[i]);
				vertices[index].SetBoneIndex(data.bone_data->bone_indices[i]);
				positions.emplace_back(std::move(data.positions[i]));
			}


			UINT i_count = static_cast<UINT>(data.incdices.size());

			for (UINT i = 0; i < i_count; ++i)
			{
				UINT index = i + index_count;
				indices[index] = data.incdices[i];
			}

			//if(false)
			if (i_count > 0)
			{
				s_mesh->AddMeshVertexInfo(lod, { i_count,index_count });
				s_mesh->SetDrawIndex(true);
			}
			else
				s_mesh->AddMeshVertexInfo(lod, { v_count,vertex_count });

			vertex_count += v_count; //다음메시의 정점의 시작위치를 정한다.
			index_count += i_count; //다음메시의 정점의 인덱스위치를 정한다.



		}
		//Blob연결

		const auto& vertex_info = s_mesh->GetVertexInfo(lod);
		if (vertex_info->CreateVertexBlob<BoneVertex>(vertex_count) == false)
		{
			LOG_ERROR("Could not create blob for vertex");
			return ;
		}
		vertex_info->CopyData(vertices.data(), vertex_count);

		const auto& index_info = s_mesh->GetIndexInfo(lod);
		if (index_info->CreateIndexBlob(index_count) == false)
		{
			LOG_ERROR("Could not create blob for index");
			return ;
		}
		index_info->CopyData(indices.data(), index_count);

		BOrientedBox box = BOrientedBox(std::move(positions));
		s_mesh->SetOrientBox(box);
	}


	bool RevLoader::LoadFrameHierArchy(std::ifstream& file, SPtr<Skeleton>& skeleton, std::vector<MeshData>& mesh_data, const std::string& path) const
	{
		std::string prefix;
		std::string parent_path = file_help::GetParentPathFromPath(path);

		//skeleton
		std::string b_name;

		int n_frame = 0; //프레임 수
		int n_childs = 0;
		int material_count = 0;
		std::string texture_name;

		int temp_int = 0;
		Mat4 temp_mat4;
		Vec3 temp_vec3;

		ReadIntager(file, n_frame);
		ReadString(file,b_name); //bone name read
		skeleton->SetBoneName(b_name);

		UINT mesh_index = 0;
		if(this != nullptr)
			 mesh_index = (m_mesh_count == 0) ? 0 : m_mesh_count - 1;
		auto& temp_data = mesh_data.at(mesh_index);

		while (ReadString(file, prefix))
		{
			switch (HashCode(prefix.c_str()))
			{
			case(HashCode("<Transform>:")):
				ReadMat4(file, temp_mat4);
				skeleton->SetTransform(temp_mat4);
				ReadVec3(file, skeleton->m_scale);
				ReadVec3(file, skeleton->m_rotation);
				ReadVec3(file, skeleton->m_translation);
				break;
			case(HashCode("<Mesh>:"))://일반 메쉬 (미처리)
				LoadMeshFromRevFile(file, mesh_data);
				if (this != nullptr) static_cast<UINT>(m_mesh_count)++;
				break;
			case(HashCode("<SkinDeformations>:")):
				if (this != nullptr)
				{
					if (m_mesh_count > 0)
					{
						mesh_data.emplace_back(std::move(MeshData{}));
						mesh_data.back().bone_data = CreateSPtr<BoneData>();
					}
					static_cast<UINT>(m_mesh_count)++;
				}
				LoadSkinDeformations(file, mesh_data.at(0).bone_data); //뼈정보는 한곳에 몰아넣기
				ReadString(file, prefix);
				if (prefix == ("<Mesh>:")) LoadMeshFromRevFile(file, mesh_data);
				break;
			case(HashCode("<Materials>:")):

				ReadIntager(file, material_count);

				while (ReadString(file,prefix))
				{
					if (prefix == ("</Materials>")) break;
					switch (HashCode(prefix.c_str()))
					{
					case HashCode("<Material>:"):
						ReadIntager(file, temp_int);
						break;
					case HashCode("<AlbedoMap>:"):
						ReadString(file, texture_name); //W_HEAD_00_violet +확장자 붙힌채로 읽기
						temp_data.mtl_names.push_back(texture_name); 
						temp_data.materials = AssetStore::LoadMaterials(parent_path + "/" + texture_name + ".mtl");
						break;
					}
				}
				break;
			case(HashCode("<Children>:")):
				ReadIntager(file, n_childs);
				if (n_childs > 0)
				{
					for (int i = 0; i < n_childs; ++i)
					{
						ReadString(file, prefix);
						if (prefix == ("<Frame>:"))
						{
							SPtr<Skeleton> child = CreateSPtr<Skeleton>();
							skeleton->SetChild(child);
							LoadFrameHierArchy(file, child, mesh_data, path);
						}
					}
				}
				break;
			case(HashCode("</Frame>")):
				if (skeleton->GetParent())return false; //한 프레임이 끝낫는데 부모가 있으면 루트가아니므로 재귀 탈출
				else break;//루트 프레임이면 반복문을 탈출하고 읽은 정보를 저장한다.
				break;
			}
		}

		return true;
	}

	void RevLoader::LoadMeshFromRevFile(std::ifstream& file, std::vector<MeshData>& mesh_data) const
	{
		std::string mesh_name;
		ReadString(file, mesh_name);

		MeshData temp_mdata;

		std::string prefix;
		Vec3 temp_vec3;
		Vec2 temp_vec2;
		UINT temp_uint;
		int temp_int;
		BOrientedBox temp_box;

		UINT mesh_index = 0;
		if(this != nullptr)
			mesh_index= (m_mesh_count == 0) ? 0 : m_mesh_count - 1;
		auto& temp_data = mesh_data.at(mesh_index);

		while (ReadString(file, prefix))
		{
			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<Bounds>:"):
				ReadVec3(file, temp_vec3);
				temp_box.SetCenter(temp_vec3);
				ReadVec3(file, temp_vec3);
				temp_box.SetExtents(temp_vec3);
				temp_data.oriented_boxes.emplace_back(std::move(temp_box));
				break;

			case HashCode("<ControlPoints>:"):
				ReadUINT(file,temp_uint);
				//vertex_count 정점수를 받는 변수 저장은 하지 않음
				// 벡터 positions 크기와 같으니까

				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
					{
						ReadVec3(file, temp_vec3);
						temp_data.positions.emplace_back(std::move(temp_vec3));
					}
				}
				break;

			case HashCode("<TextureCoords0>:"):
				ReadUINT(file, temp_uint);
				//tex_coord_count
				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
					{
						ReadVec2(file,temp_vec2);
						temp_data.tex_coords.emplace_back(std::move(temp_vec2));
					}
				}
				break;

			case HashCode("<TextureCoords1>:"):
				ReadUINT(file, temp_uint);
				//tex_coord2_count
				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
					{
						ReadVec2(file, temp_vec2);
						// temp_data.tex_coords2.emplace_back(std::move(temp_vec2));
					}
				}
				break;

			case HashCode("<Normals>:"):
				ReadUINT(file, temp_uint);
				//normal_count
				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
					{
						ReadVec3(file, temp_vec3);
						temp_data.normals.emplace_back(std::move(temp_vec3));
					}
				}
				break;

			case HashCode("<Tangents>:"):
				ReadUINT(file, temp_uint);
				//tan_count
				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
					//tangent.emplace_back
				}
				break;

			case HashCode("<BiTangents>:"):
				ReadUINT(file, temp_uint);
				//bitan_count
				if (temp_uint > 0)
				{
					for (UINT i = 0; i < temp_uint; ++i)
						ReadVec3(file, temp_vec3);
					//bitangent.emplace_back
				}
				break;

			case HashCode("<Polygons>:"):
				ReadIntager(file, temp_int);
				//Polgon_count
				while (ReadString(file, prefix))
				{
					if (prefix == ("</Polygons>")) break;
					switch (HashCode(prefix.c_str()))
					{
					case HashCode("<SubIndices>:"):
					{
						ReadIntager(file, temp_int);//index_count
						ReadIntager(file, temp_int);
						int sub_mesh_count = temp_int;
						//if (sub_mesh_count == 0) sub_mesh_count = 1;

						while (ReadString(file, prefix))
						{
							if (prefix == ("</Polygons>")) break;
							switch (HashCode(prefix.c_str()))
							{
							case HashCode("<SubIndex>:"):
							{
								ReadIntager(file, temp_int);//index_count
								ReadIntager(file, temp_int);
								int subset_index_size = temp_int;
								if (subset_index_size > 0)
								{
									for (int i = 0; i < subset_index_size; i++)
									{
										ReadUINT(file, temp_uint);
										temp_data.incdices.emplace_back(std::move(temp_uint));
									}
								}
							}
							break;

							default:
								break;
							}
						}
					}
					break;

					default:
						break;
					}
					if (prefix == ("</Polygons>"))break;

				}
				break;

			case HashCode("</Mesh>"):
				return;
			}
		}
	}

	void RevLoader::LoadSkinDeformations(std::ifstream& file, SPtr<BoneData>& bone_data) const
	{
		std::string prefix;

		Mat4 temp_mat4;
		Vec3 temp_vec3;
		Vec4 temp_vec4;
		IVec4 temp_ivec4;
		int temp_int;

		int skinning_bone_count = 0;
		int bone_per_vertex = 4;

		BOrientedBox temp_box;

		while (ReadString(file, prefix))
		{
			if (prefix== ("</SkinDeformations>")) break;

			switch (HashCode(prefix.c_str()))
			{
			case HashCode("<BonesPerVertex>:"):
				ReadIntager(file, bone_per_vertex);
				break;
			case HashCode("<Bounds>:"):
				ReadVec3(file, temp_vec3);
				temp_box.SetCenter(temp_vec3);
				ReadVec3(file, temp_vec3);
				temp_box.SetExtents(temp_vec3);
				bone_data->oriented_boxes.emplace_back(std::move(temp_box));
				break;
			case HashCode("<BoneNames>:"):
				ReadIntager(file, skinning_bone_count);
				if (skinning_bone_count > 0)
				{
					for (int i = 0; i < skinning_bone_count; ++i)
					{
						ReadString(file, prefix);
						bone_data->bone_names.emplace_back(std::move(prefix));
					}
				}
				break;
			case HashCode("<BoneOffsets>:"):
				ReadIntager(file, skinning_bone_count);
				if (skinning_bone_count > 0)
				{
					for (int i = 0; i < skinning_bone_count; ++i)
					{
						ReadMat4(file, temp_mat4);
						bone_data->bone_offsets.emplace_back(std::move(temp_mat4));
					}
				}
				break;
			case HashCode("<BoneIndices>:"):
				ReadIntager(file, temp_int);
				if (temp_int > 0)
				{
					for (int i = 0; i < temp_int; ++i)
					{
						ReadIVec4(file, temp_ivec4);
						bone_data->bone_indices.emplace_back(std::move(temp_ivec4));
					}
				}
				break;
			case HashCode("<BoneWeights>:"):
				ReadIntager(file, temp_int);
				if (temp_int > 0)
				{
					for (int i = 0; i < temp_int; ++i)
					{
						ReadVec4(file, temp_vec4);
						bone_data->bone_weights.emplace_back(std::move(temp_vec4));
					}
				}
				break;
			default:
				break;
			}
		}
	}

	void RevLoader::InitializeMeshData(std::vector<MeshData>& mesh_data) const
	{
		mesh_data.emplace_back(std::move(MeshData{}));
		mesh_data.back().bone_data = CreateSPtr<BoneData>();
	}

	namespace load_help
	{
		int ReadString(std::ifstream& file, std::string& str)
		{
			int length;
			file.read((char*)&length, sizeof(int));
			if (length > 0)
			{
				std::vector<char> buffer(length);
				file.read(buffer.data(), length * sizeof(char));
				str.clear();
				str.assign(buffer.begin(), buffer.end());
				buffer.clear();
			}
			return length;
		}
		int ReadString(std::ifstream& file, int length, std::string& str)
		{
			if (length > 0)
			{
				std::vector<char> buffer(length);
				file.read(buffer.data(), length * sizeof(char));
				str.clear();
				str.assign(buffer.begin(), buffer.end());
				buffer.clear();
			}
			return length;
		}
		void ReadUINT(std::ifstream& file, UINT& value)
		{
			file.read((char*)&value, sizeof(UINT));
		}
		void ReadIntager(std::ifstream& file, int& value)
		{
			file.read((char*)&value, sizeof(int));
		}
		void ReadFloat(std::ifstream& file, float& value)
		{
			file.read((char*)&value, sizeof(float));
		}
		void ReadMat4(std::ifstream& file, Mat4& mat4)
		{
			file.read((char*)&mat4, sizeof(Mat4));
		}

		void ReadVec2(std::ifstream& file, Vec2& vec2)
		{
			file.read((char*)&vec2, sizeof(Vec2));

		}
		void ReadVec3(std::ifstream& file, Vec3& vec3)
		{
			file.read((char*)&vec3, sizeof(Vec3));
		}
		void ReadVec4(std::ifstream& file, Vec4& vec4)
		{
			file.read((char*)&vec4, sizeof(Vec4));
		}
		void ReadIVec4(std::ifstream& file, IVec4& ivec4)
		{
			file.read((char*)&ivec4, sizeof(IVec4));
		}
	}
}
