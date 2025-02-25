#include "stdafx.h"
#include "client/renderer/renderitem/mesh_render_item.h"
#include "client/renderer/core/render.h"
#include "client/renderer/frameresource/core/frame_resource_manager.h"
#include "client/renderer/frameresource/core/frame_resource.h"
#include "client/renderer/frameresource/mesh_frame_resource.h"
#include "client/asset/mesh/mesh.h"
#include "client/object/actor/core/actor.h"
#include "client/object/component/mesh/core/mesh_component.h"
#include "client/object/component/mesh/static_mesh_component.h"
#include "client/object/component/mesh/skeletal_mesh_component.h"
#include "client/util/d3d_util.h"
#include "client/util/upload_buffer.h"

namespace client_fw
{
	MeshRenderItem::MeshRenderItem(const std::string& owner_shader_name)
		: RenderItem(owner_shader_name)
	{
	}

	StaticMeshRenderItem::StaticMeshRenderItem(const std::string& owner_shader_name)
		: MeshRenderItem(owner_shader_name)
	{
	}

	StaticMeshRenderItem::~StaticMeshRenderItem()
	{
	}

	void StaticMeshRenderItem::Initialize(ID3D12Device* device, const std::vector<eRenderLevelType>& level_types)
	{
		const auto& frame_resources = FrameResourceManager::GetManager().GetFrameResources();
		for (const auto& frame : frame_resources)
		{
			for (eRenderLevelType level_type : level_types)
				frame->CreateStaticMeshFrameResource(device, m_owner_shader_name, level_type);
		}
	}

	void StaticMeshRenderItem::Update(ID3D12Device* device, eRenderLevelType level_type)
	{
		MeshesInstanceDrawInfo instance_info;
		//카메라마다의 시작 위치 저장
		instance_info.start_index = static_cast<UINT>(m_meshes_instance_data[level_type].size());

		UINT mesh_start_index_for_camera = 0;
		for (const auto& mesh_data : m_mesh_data)
		{
			MeshDrawInfo info;
			info.mesh = mesh_data->mesh;
			info.num_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);
			info.start_index_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);

			if (info.mesh->IsVisible() == false)
				continue;

			std::vector<std::vector<RSInstanceData>> instance_data(mesh_data->mesh->GetLODCount());
			for (auto& data : instance_data)
				data.reserve(m_meshes_instance_data[level_type].capacity());

			for (const auto& mesh_comp : mesh_data->mesh_comps)
			{
				if (mesh_comp->IsVisible())
				{
					UINT lod = mesh_comp->GetLevelOfDetail();
					instance_data[lod].emplace_back(RSInstanceData{ mesh_comp->GetWorldTransposeMatrix(), mesh_comp->GetWorldInverseMatrix() });

					mesh_comp->SetVisiblity(false);
				}
			}

			UINT mesh_count = 0;

			for (UINT lod = 0; lod < mesh_data->mesh->GetLODCount(); ++lod)
			{
				info.start_index_of_lod_instance_data[lod] = mesh_count;
				mesh_count += static_cast<UINT>(instance_data[lod].size());
				info.num_of_lod_instance_data[lod] = static_cast<UINT>(instance_data[lod].size());
			}

			info.draw_start_index = mesh_start_index_for_camera;
			mesh_start_index_for_camera += mesh_count;

			for (auto& data : instance_data)
				std::move(data.begin(), data.end(), std::back_inserter(m_meshes_instance_data[level_type]));

			mesh_data->mesh->SetVisible(false);
			instance_info.mesh_draw_infos.emplace_back(std::move(info));
		}

		//mesh start index for camera는 mesh마다 그려지는 count를 더했기 때문에 최종적으로는 카메라가 그릴 데이터의 수가 된다.
		instance_info.num_of_instnace_data = static_cast<UINT>(mesh_start_index_for_camera);

		const auto& mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetStaticMeshFrameResource(m_owner_shader_name, level_type);
		mesh_resource->AddMeshesInstanceDrawInfo(std::move(instance_info));
	}

	void StaticMeshRenderItem::UpdateFrameResource(ID3D12Device* device, eRenderLevelType level_type)
	{
		const auto& mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetStaticMeshFrameResource(m_owner_shader_name, level_type);

		UINT new_size = static_cast<UINT>(m_meshes_instance_data[level_type].size());
		if (new_size > 0)
		{
			UINT mesh_instance_size = mesh_resource->GetSizeOfInstanceData();
			bool is_need_resource_create = false;

			while (mesh_instance_size <= new_size)
			{
				mesh_instance_size = static_cast<UINT>(roundf(static_cast<float>(mesh_instance_size) * 1.5f));
				is_need_resource_create = true;
			}

			if (is_need_resource_create)
			{
				mesh_resource->GetInstanceData()->CreateResource(device, mesh_instance_size);
				mesh_resource->SetSizeOfInstanceData(mesh_instance_size);
			}

			mesh_resource->GetInstanceData()->CopyVectorData(std::move(m_meshes_instance_data[level_type]));

			m_meshes_instance_data[level_type].clear();
		}
	}

	void StaticMeshRenderItem::Draw(ID3D12GraphicsCommandList* command_list, eRenderLevelType level_type,
		std::function<void()>&& draw_function) const
	{
		const auto& mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetStaticMeshFrameResource(m_owner_shader_name, level_type);

		MeshesInstanceDrawInfo instance_info = mesh_resource->GetMeshesInstanceDrawInfo();

		if (instance_info.num_of_instnace_data > 0)
		{
			draw_function();

			const auto& instance_data = mesh_resource->GetInstanceData();

			for (const auto& mesh_info : instance_info.mesh_draw_infos)
			{
				mesh_info.mesh->PreDraw(command_list);

				for (UINT lod = 0; lod < mesh_info.mesh->GetLODCount(); ++lod)
				{
					if (mesh_info.num_of_lod_instance_data[lod] > 0)
					{
						//
						// 최종적으로 그릴 때는 카메라 마다의 시작 위치인 (instance_info.start_index) + 
						// 각 카메라가 그리는 mesh의 정보 시작 위치 (mesh_info.draw_start_index) +
						// 각 mesh의 lod의 시작 위치 (mesh_info.start_index_of_lod_instance_data) 를 더한 값을
						// 지정한다. {설명 참고 : MeshesInstanceDrawInfo,  MeshDrawInfo
						//
						command_list->SetGraphicsRootShaderResourceView(1, instance_data->GetResource()->GetGPUVirtualAddress() +
							(instance_info.start_index + mesh_info.draw_start_index + mesh_info.start_index_of_lod_instance_data[lod]) *
							instance_data->GetByteSize());

						// 개선될 수 있는 여지가 있음
						if(level_type == eRenderLevelType::kShadow || level_type == eRenderLevelType::kShadowCube)
							mesh_info.mesh->DrawForShadow(command_list, mesh_info.num_of_lod_instance_data[lod], lod);
						else
							mesh_info.mesh->Draw(command_list, mesh_info.num_of_lod_instance_data[lod], lod);
					}
				}
			}
		}
	}

	void StaticMeshRenderItem::RegisterMeshComponent(const SPtr<MeshComponent>& mesh_comp)
	{
		std::string path = mesh_comp->GetMesh()->GetPath();

		if (m_mesh_data_map.find(path) != m_mesh_data_map.cend())
		{
			mesh_comp->SetRenderItemIndex(static_cast<UINT>(m_mesh_data_map[path]->mesh_comps.size()));
			m_mesh_data_map[path]->mesh_comps.push_back(std::static_pointer_cast<StaticMeshComponent>(mesh_comp));
		}
		else
		{
			SPtr<StaticMeshData> mesh_data = CreateSPtr<StaticMeshData>();
			mesh_data->mesh = mesh_comp->GetMesh();
			mesh_comp->SetRenderItemIndex(0);
			mesh_data->mesh_comps.push_back(std::static_pointer_cast<StaticMeshComponent>(mesh_comp));

			m_mesh_data.push_back(mesh_data);
			m_mesh_data_map.insert({ path, mesh_data });
		}
	}

	void StaticMeshRenderItem::UnregisterMeshComponent(const SPtr<MeshComponent>& mesh_comp)
	{
		std::string path = mesh_comp->GetMesh()->GetPath();

		if (m_mesh_data_map.find(path) != m_mesh_data_map.cend())
		{
			UINT index = mesh_comp->GetRenderItemIndex();

			auto& mesh_data = m_mesh_data_map[path];

			std::swap(*(mesh_data->mesh_comps.begin() + index), *(mesh_data->mesh_comps.end() - 1));
			mesh_data->mesh_comps[index]->SetRenderItemIndex(index);
			mesh_data->mesh_comps.pop_back();
		}
	}

	SkeletalMeshRenderItem::SkeletalMeshRenderItem(const std::string& owner_shader_name)
		:MeshRenderItem(owner_shader_name)
	{
	}

	SkeletalMeshRenderItem::~SkeletalMeshRenderItem()
	{
	}

	void SkeletalMeshRenderItem::Initialize(ID3D12Device* device, const std::vector<eRenderLevelType>& level_types)
	{
		const auto& frame_resources = FrameResourceManager::GetManager().GetFrameResources();
		for (const auto& frame : frame_resources)
		{
			for (eRenderLevelType level_type : level_types)
				frame->CreateSkeletalMeshFrameResource(device, m_owner_shader_name, level_type);
		}
	}

	void SkeletalMeshRenderItem::Update(ID3D12Device* device, eRenderLevelType level_type)
	{
		MeshesInstanceDrawInfo instance_info;
		instance_info.start_index = static_cast<UINT>(m_skeletal_meshes_instance_data[level_type].size());

		UINT bone_start_index_for_camera = 0;
		UINT mesh_start_index_for_camera = 0;
		for (const auto& mesh_data : m_skeletal_mesh_data)
		{
			MeshDrawInfo info;
			info.mesh = mesh_data->mesh;
			info.num_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);
			info.start_index_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);

			if (info.mesh->IsVisible() == false)
				continue;

			std::vector<std::vector<RSInstanceData>> instance_data(mesh_data->mesh->GetLODCount());
			std::vector<std::vector<RSSkeletalInstanceData>> skeletal_transform_data(mesh_data->mesh->GetLODCount());
			UINT bone_count = mesh_data->mesh_comps[0]->GetSkeletalMesh()->GetBoneCount();
			UINT bone_start_index = 0;

			for (const auto& mesh_comp : mesh_data->mesh_comps)
			{
				if (mesh_comp->IsVisible())
				{
					UINT lod = mesh_comp->GetLevelOfDetail();

					const auto& bone_transform_data = mesh_comp->GetBoneTransformData();
					for (UINT index = 0; index < bone_count; ++index) 
						skeletal_transform_data[lod].emplace_back(RSSkeletalInstanceData{ bone_transform_data[index] });
					instance_data[lod].emplace_back(RSInstanceData{ mesh_comp->GetWorldTransposeMatrix(), mesh_comp->GetWorldInverseMatrix(),
						bone_start_index_for_camera + bone_start_index });

					bone_start_index += bone_count;

					mesh_comp->SetVisiblity(false);
				}
			}

			UINT mesh_count = 0;

			for (UINT lod = 0; lod < mesh_data->mesh->GetLODCount(); ++lod)
			{
				info.start_index_of_lod_instance_data[lod] = mesh_count;
				mesh_count += static_cast<UINT>(instance_data[lod].size());
				info.num_of_lod_instance_data[lod] = static_cast<UINT>(instance_data[lod].size());
			}

			info.draw_start_index = mesh_start_index_for_camera;
			mesh_start_index_for_camera += mesh_count;
			bone_start_index_for_camera += bone_count * mesh_count;

			for (auto& data : instance_data)
				std::move(data.begin(), data.end(), std::back_inserter(m_skeletal_meshes_instance_data[level_type]));
			for(auto& data : skeletal_transform_data)
				std::move(data.begin(), data.end(), std::back_inserter(m_skeletal_transforms_data[level_type]));

			mesh_data->mesh->SetVisible(false);
			instance_info.mesh_draw_infos.emplace_back(std::move(info));
		}

		instance_info.num_of_instnace_data = static_cast<UINT>(mesh_start_index_for_camera);

		const auto& mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetSkeletalMeshFrameResource(m_owner_shader_name, level_type);
		mesh_resource->AddMeshesInstanceDrawInfo(std::move(instance_info));
	}

	void SkeletalMeshRenderItem::UpdateFrameResource(ID3D12Device* device, eRenderLevelType level_type)
	{
		const auto& skeletal_mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetSkeletalMeshFrameResource(m_owner_shader_name, level_type);

		UINT new_size = static_cast<UINT>(m_skeletal_meshes_instance_data[level_type].size());

		if (new_size > 0)
		{
			UINT skeletal_mesh_instance_size = skeletal_mesh_resource->GetSizeOfInstanceData();
			bool is_need_skeletal_mesh_resource_create = false;

			while (skeletal_mesh_instance_size <= new_size)
			{
				skeletal_mesh_instance_size = static_cast<UINT>(roundf(static_cast<float>(skeletal_mesh_instance_size) * 1.5f));
				is_need_skeletal_mesh_resource_create = true;
			}

			if (is_need_skeletal_mesh_resource_create)
			{
				
				skeletal_mesh_resource->GetInstanceData()->CreateResource(device, skeletal_mesh_instance_size);
				skeletal_mesh_resource->SetSizeOfInstanceData(skeletal_mesh_instance_size);
			}

			skeletal_mesh_resource->GetInstanceData()->CopyVectorData(std::move(m_skeletal_meshes_instance_data[level_type]));
			m_skeletal_meshes_instance_data[level_type].clear();
		}

		new_size = static_cast<UINT>(m_skeletal_transforms_data[level_type].size());

		if (new_size > 0)
		{
			UINT skeletal_transfrom_size = skeletal_mesh_resource->GetSizeOfSkeletalTransformData();
			bool is_need_skeletal_transform_resource_create = false;

			while (skeletal_transfrom_size <= new_size)
			{
				skeletal_transfrom_size = static_cast<UINT>(roundf(static_cast<float>(skeletal_transfrom_size) * 1.5f));
				is_need_skeletal_transform_resource_create = true;
			}

			if (is_need_skeletal_transform_resource_create)
			{
				skeletal_mesh_resource->GetSkeletalTransformData()->CreateResource(device, skeletal_transfrom_size);
				skeletal_mesh_resource->SetSizeOfSkeletalTransformData(skeletal_transfrom_size);
			}

			skeletal_mesh_resource->GetSkeletalTransformData()->CopyVectorData(std::move(m_skeletal_transforms_data[level_type]));
			m_skeletal_transforms_data[level_type].clear();
		}
	}

	void SkeletalMeshRenderItem::Draw(ID3D12GraphicsCommandList* command_list, eRenderLevelType level_type,
		std::function<void()>&& draw_function) const
	{
		const auto& skeletal_mesh_resource = FrameResourceManager::GetManager().
			GetCurrentFrameResource()->GetSkeletalMeshFrameResource(m_owner_shader_name, level_type);

		MeshesInstanceDrawInfo instance_info = skeletal_mesh_resource->GetMeshesInstanceDrawInfo();

		if (instance_info.num_of_instnace_data > 0)
		{
			draw_function();

			const auto& instance_data = skeletal_mesh_resource->GetInstanceData();
			const auto& skeletal_transform_data = skeletal_mesh_resource->GetSkeletalTransformData();

			command_list->SetGraphicsRootShaderResourceView(7, skeletal_transform_data->GetResource()->GetGPUVirtualAddress());

			for (const auto& mesh_info : instance_info.mesh_draw_infos)
			{
				mesh_info.mesh->PreDraw(command_list);

				for (UINT lod = 0; lod < mesh_info.mesh->GetLODCount(); ++lod)
				{
					if (mesh_info.num_of_lod_instance_data[lod] > 0)
					{
						command_list->SetGraphicsRootShaderResourceView(1, instance_data->GetResource()->GetGPUVirtualAddress() +
							(instance_info.start_index + mesh_info.draw_start_index + mesh_info.start_index_of_lod_instance_data[lod]) *
							instance_data->GetByteSize());

						if (level_type == eRenderLevelType::kShadow || level_type == eRenderLevelType::kShadowCube)
							mesh_info.mesh->DrawForShadow(command_list, mesh_info.num_of_lod_instance_data[lod], lod);
						else
							mesh_info.mesh->Draw(command_list, mesh_info.num_of_lod_instance_data[lod], lod);
					}
				}
			}
		}
	}

	void SkeletalMeshRenderItem::RegisterMeshComponent(const SPtr<MeshComponent>& mesh_comp)
	{
		std::string path = mesh_comp->GetMesh()->GetPath();

		if (m_skeletal_mesh_data_map.find(path) != m_skeletal_mesh_data_map.cend())
		{
			mesh_comp->SetRenderItemIndex(static_cast<UINT>(m_skeletal_mesh_data_map[path]->mesh_comps.size()));
			m_skeletal_mesh_data_map[path]->mesh_comps.push_back(std::static_pointer_cast<SkeletalMeshComponent>(mesh_comp));
		}
		else
		{
			SPtr<SkeletalMeshData> mesh_data = CreateSPtr<SkeletalMeshData>();
			mesh_data->mesh = mesh_comp->GetMesh();
			mesh_comp->SetRenderItemIndex(0);
			mesh_data->mesh_comps.push_back(std::static_pointer_cast<SkeletalMeshComponent>(mesh_comp));

			m_skeletal_mesh_data.push_back(mesh_data);
			m_skeletal_mesh_data_map.insert({ path, mesh_data });
		}
	}

	void SkeletalMeshRenderItem::UnregisterMeshComponent(const SPtr<MeshComponent>& mesh_comp)
	{
		std::string path = mesh_comp->GetMesh()->GetPath();

		if (m_skeletal_mesh_data_map.find(path) != m_skeletal_mesh_data_map.cend())
		{
			UINT index = mesh_comp->GetRenderItemIndex();

			auto& mesh_data = m_skeletal_mesh_data_map[path];

			std::swap(*(mesh_data->mesh_comps.begin() + index), *(mesh_data->mesh_comps.end() - 1));
			mesh_data->mesh_comps[index]->SetRenderItemIndex(index);
			mesh_data->mesh_comps.pop_back();
		}
	}

}

