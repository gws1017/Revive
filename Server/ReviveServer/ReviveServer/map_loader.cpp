#include"pch.h"
#include"map_loader.h"
#include<fstream>
#include<sstream>
#include<vector>
#include"map_object.h"
#include<unordered_map>
#include"define.h"
using namespace std;

void MapLoader::LoadMap(const std::string& path)
{
	ifstream map_file(path, ifstream::binary);
	if (!map_file)
	{
		cout << "맵 파일 불러오기 실패" << endl;
		return;
	}
	vector<ActorInfo>actor_info_data;
	actor_info_data.reserve(1000);
	stringstream ss;
	string line;
	string prefix;
	string temp_name;
	UINT temp_mesh_count, temp_col_count;
	Vector3 temp_pos;
	std::vector<Vector3> scales;
	std::vector<Vector3> collision_centers;
	std::vector<Vector3> collision_extents;
	std::vector<Vector3> positions;
	
	while (getline(map_file, line))
	{
		ss.clear();
		prefix.clear();
		ss.str(line);
		ss >> prefix;
		switch (HashCode(prefix.c_str()))
		{
		case HashCode("ActorName"):
		{
			ss >> temp_name >> temp_mesh_count >> temp_col_count;
			ss >> temp_pos.x >> temp_pos.y >> temp_pos.z;
			actor_info_data.emplace_back(temp_name, temp_pos, 
				temp_mesh_count, temp_col_count);
			break;
		}
		case HashCode("BoxCollision"):
		{
			ss >> temp_name;//name vector info
			ss >> temp_name >> temp_pos.x >> temp_pos.y >> temp_pos.z;
			collision_centers.emplace_back(std::move(temp_pos));
			ss >> temp_name >> temp_pos.x >> temp_pos.y >> temp_pos.z;
			collision_extents.emplace_back(std::move(temp_pos));
			break;
		}
		case HashCode("FilePath"): {
			break;
		}
		case HashCode("Position"): {
			ss >> temp_pos.x >> temp_pos.y >> temp_pos.z;
			positions.emplace_back(std::move(temp_pos));
			break;
		}
		case HashCode("Rotation"): {

			break;
		}
		case HashCode("Scale"): {
			ss >> temp_pos.x >> temp_pos.y >> temp_pos.z;
			scales.emplace_back(std::move(temp_pos));
			break;
		}
		}
	}
	int col_index{ 0 };
	for (auto& act_info : actor_info_data)
	{
		if (act_info.collision_count <=0)
			continue;
		switch (HashCode(act_info.name.c_str()))
		{
			//물체들
		case HashCode("Base"):
		case HashCode("Fence"):
		case HashCode("Wall"):
		{
			for (int i = col_index; i < col_index + act_info.collision_count; ++i)
			{
				//map_obj_manager 만들기
				Vector3 pos{ act_info.position + collision_centers[i] };
				if (m_map_objects.find(i) == m_map_objects.end())
					m_map_objects[i] = new MapObj(i, pos, collision_extents[i], true);
				else
					cout << "있는애가 또옴" << endl;
			}
		}
		break;
		//영역들
		case HashCode("ActivityArea"):
		case HashCode("SpawnArea"):
		{
			for (int i = col_index; i < col_index + act_info.collision_count; ++i)
			{
				//map_obj_manager 만들기
				
				Vector3 pos{ act_info.position + collision_centers[i] };
				if (m_map_objects.find(i) == m_map_objects.end())
					m_map_objects[i] = new MapObj(i, pos, collision_extents[i], false);
				else
					cout << "있는애가 또옴" << endl;
			}
		}
		break;
		}
		col_index += act_info.collision_count;
		//MapObj추가
	}
	
	
}
