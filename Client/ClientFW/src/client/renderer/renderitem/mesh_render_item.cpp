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
	StaticMeshRenderItem::StaticMeshRenderItem()
		: MeshRenderItem()
	{
	}

	StaticMeshRenderItem::~StaticMeshRenderItem()
	{
	}

	void StaticMeshRenderItem::Update(ID3D12Device* device)
	{
		MeshesInstanceDrawInfo instance_info;
		instance_info.start_index = static_cast<UINT>(m_meshes_instance_data.size());

		UINT start_index = 0;
		for (const auto& mesh_data : m_mesh_data)
		{
			MeshDrawInfo info;
			info.mesh = mesh_data->mesh;
			info.num_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);
			info.start_index_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);

			UINT mesh_count = 0;

			for (UINT lod = 0; lod < mesh_data->mesh->GetLODCount(); ++lod)
			{
				mesh_count += mesh_data->mesh->GetLODMeshCount(lod);
				info.num_of_lod_instance_data[lod] = mesh_data->mesh->GetLODMeshCount(lod);
				info.start_index_of_lod_instance_data[lod] = mesh_count;
			}

			if (mesh_count == 0)
				continue;

			info.draw_start_index = start_index;
			start_index += mesh_count;

			std::vector<RSInstanceData> instance_data(mesh_count);

			for (const auto& mesh_comp : mesh_data->mesh_comps)
			{
				if (mesh_comp->IsVisible())
				{
					UINT lod = mesh_comp->GetLevelOfDetail();

					instance_data[--(info.start_index_of_lod_instance_data.at(lod))] =
						RSInstanceData{ mesh_comp->GetWorldTransposeMatrix(), mesh_comp->GetWorldInverseMatrix() };

					mesh_comp->SetVisiblity(false);
				}
			}

			std::move(instance_data.begin(), instance_data.end(), std::back_inserter(m_meshes_instance_data));

			mesh_data->mesh->ResetLOD();
			instance_info.mesh_draw_infos.emplace_back(std::move(info));
		}

		instance_info.num_of_instnace_data = static_cast<UINT>(start_index);

		const auto& mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetStaticMeshFrameResource();
		mesh_resource->AddMeshesInstanceDrawInfo(std::move(instance_info));
	}

	void StaticMeshRenderItem::UpdateFrameResource(ID3D12Device* device)
	{
		const auto& mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetStaticMeshFrameResource();

		UINT new_size = static_cast<UINT>(m_meshes_instance_data.size());
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

			UINT index = 0;
			for (const auto& instance_data : m_meshes_instance_data)
				mesh_resource->GetInstanceData()->CopyData(index++, instance_data);

			m_meshes_instance_data.clear();
		}
	}

	void StaticMeshRenderItem::Draw(ID3D12GraphicsCommandList* command_list) const
	{
		const auto& mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetStaticMeshFrameResource();
		MeshesInstanceDrawInfo instance_info = mesh_resource->GetMeshesInstanceDrawInfo();

		if (instance_info.num_of_instnace_data > 0)
		{
			const auto& instance_data = mesh_resource->GetInstanceData();

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

	SkeletalMeshRenderItem::SkeletalMeshRenderItem()
		:MeshRenderItem()
	{
	}

	SkeletalMeshRenderItem::~SkeletalMeshRenderItem()
	{
	}

	void SkeletalMeshRenderItem::Update(ID3D12Device* device)
	{
		MeshesInstanceDrawInfo instance_info;
		instance_info.start_index = static_cast<UINT>(m_skeletal_meshes_instance_data.size());

		UINT start_index = 0;

		for (const auto& mesh_data : m_skeletal_mesh_data)
		{
			MeshDrawInfo info;
			info.mesh = mesh_data->mesh;
			info.num_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);
			info.start_index_of_lod_instance_data.resize(mesh_data->mesh->GetLODCount(), 0);

			UINT mesh_count = 0;

			for (UINT lod = 0; lod < mesh_data->mesh->GetLODCount(); ++lod)
			{
				mesh_count += mesh_data->mesh->GetLODMeshCount(lod);
				info.num_of_lod_instance_data[lod] = mesh_data->mesh->GetLODMeshCount(lod);
				info.start_index_of_lod_instance_data[lod] = mesh_count;
			}

			if (mesh_count == 0)
				continue;

			info.draw_start_index = start_index;
			start_index += mesh_count;

			std::vector<RSSkeletalInstanceData> instance_data(mesh_count);

			for (const auto& mesh_comp : mesh_data->mesh_comps)
			{
				if (mesh_comp->IsVisible())
				{
					UINT lod = mesh_comp->GetLevelOfDetail();
					UINT bone_count = mesh_count * mesh_comp->GetSkeletalMesh()->GetBoneCount();
					instance_data[--(info.start_index_of_lod_instance_data.at(lod))] =
						RSSkeletalInstanceData{ mesh_comp->GetWorldTransposeMatrix(), mesh_comp->GetWorldInverseMatrix(),bone_count };

					mesh_comp->SetVisiblity(false);
				}
			}

			std::move(instance_data.begin(), instance_data.end(), std::back_inserter(m_skeletal_meshes_instance_data));

			mesh_data->mesh->ResetLOD();
			instance_info.mesh_draw_infos.emplace_back(std::move(info));
		}

		instance_info.num_of_instnace_data = static_cast<UINT>(start_index);

		const auto& mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetSkeletalMeshFrameResource();
		mesh_resource->AddMeshesInstanceDrawInfo(std::move(instance_info));
	}

	void SkeletalMeshRenderItem::UpdateFrameResource(ID3D12Device* device)
	{
		const auto& skeletal_mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetSkeletalMeshFrameResource();

		UINT new_size = static_cast<UINT>(m_skeletal_meshes_instance_data.size());
		if (new_size > 0)
		{
			UINT skeletal_mesh_instance_size = skeletal_mesh_resource->GetSizeOfInstanceData();
			bool is_need_resource_create = false;

			while (skeletal_mesh_instance_size <= new_size)
			{
				skeletal_mesh_instance_size = static_cast<UINT>(roundf(static_cast<float>(skeletal_mesh_instance_size) * 1.5f));
				is_need_resource_create = true;
			}

			if (is_need_resource_create)
			{
				skeletal_mesh_resource->GetInstanceData()->CreateResource(device, skeletal_mesh_instance_size);
				skeletal_mesh_resource->SetSizeOfInstanceData(skeletal_mesh_instance_size);
			}

			UINT index = 0;
			for (const auto& instance_data : m_skeletal_meshes_instance_data)
				skeletal_mesh_resource->GetInstanceData()->CopyData(index++, instance_data);

			m_skeletal_mesh_data.clear();
		}
	}

	void SkeletalMeshRenderItem::Draw(ID3D12GraphicsCommandList* command_list) const
	{
		const auto& skeletal_mesh_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetSkeletalMeshFrameResource();
		MeshesInstanceDrawInfo instance_info = skeletal_mesh_resource->GetMeshesInstanceDrawInfo();

		if (instance_info.num_of_instnace_data > 0)
		{
			const auto& instance_data = skeletal_mesh_resource->GetInstanceData();

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

