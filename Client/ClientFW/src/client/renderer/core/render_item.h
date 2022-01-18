#pragma once

namespace client_fw
{
	class Mesh;
	class MeshComponent;
	template<class T> class UploadBuffer;

	struct RSInstanceData
	{
		Mat4 world_matrix;
		Mat4 world_inverse_transpose;
	};

	struct MeshComponentData
	{
		SPtr<MeshComponent> mesh_comp;
		bool m_is_need_update;
	};

	class RenderItem final
	{
	public:
		RenderItem(const SPtr<Mesh>& mesh, UINT material_count);
		~RenderItem();

		void Shutdown();

		void Update(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);
		void Draw(ID3D12GraphicsCommandList* command_list);

		void CreateResources(ID3D12Device* device);
		virtual void UpdateResources();

		virtual void RegisterMeshComponent(const SPtr<MeshComponent>& mesh_comp);
		virtual void UnregisterMeshComponent(const SPtr<MeshComponent>& mesh_comp);

	protected:
		bool m_is_need_resource_create = true;
		SPtr<Mesh> m_mesh;
		UINT m_material_count = 0;
		
		std::vector<MeshComponentData> m_mesh_comp_data;
		std::unordered_set<UINT> m_changed_resource_index;

		UPtr<UploadBuffer<RSInstanceData>> m_instance_data;

		UINT m_num_of_instance_data = 1;

	public:
		const SPtr<Mesh>& GetMesh() const { return m_mesh; }
	};
}
