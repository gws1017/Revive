#pragma once

namespace client_fw
{
	template<class VertexType>
	class UploadPrimitive;
	class BillboardVertex;

	struct BillboardDrawInfo
	{
		UINT start_index;
		UINT num_of_draw_data;
		UINT fix_up_start_index;
		UINT num_of_draw_fix_up_data;
	};

	class BillboardFrameResource
	{
	public:
		BillboardFrameResource();
		~BillboardFrameResource();

		bool Initialize(ID3D12Device* device);
		void Shutdown();

	private:
		UPtr<UploadPrimitive<BillboardVertex>> m_texture_billboard_primitive;
		UPtr<UploadPrimitive<BillboardVertex>> m_material_billboard_primitive;

		UINT m_size_of_texture_billboard_primitive = 1;
		UINT m_size_of_material_billboard_primitive = 1;

		std::queue<BillboardDrawInfo> m_texture_draw_info;
		std::queue<BillboardDrawInfo> m_material_draw_info;

	public:
		const UPtr<UploadPrimitive<BillboardVertex>>& GetTextureBillboardPrimitive() const { return m_texture_billboard_primitive; }
		const UPtr<UploadPrimitive<BillboardVertex>>& GetMaterialBillboardPrimitive() const { return m_material_billboard_primitive; }

		UINT GetSizeOfTextureBillboardPrimitive() const { return m_size_of_texture_billboard_primitive; }
		UINT GetSizeOfMaterialBillboardPrimitive() const { return m_size_of_material_billboard_primitive; }
		void SetSizeOfTextureBillboardPrimitive(UINT value) { m_size_of_texture_billboard_primitive = value; }
		void SetSizeOfMaterialBillboardPrimitive(UINT value) { m_size_of_material_billboard_primitive = value; }

		void AddTextureBillboardDrawInfo(BillboardDrawInfo&& info) { m_texture_draw_info.emplace(std::move(info)); }
		void AddMaterialBillboardDrawInfo(BillboardDrawInfo&& info) { m_material_draw_info.emplace(std::move(info)); }

		BillboardDrawInfo GetTextureBillboardDrawInfo();
		BillboardDrawInfo GetMaterialBillboardDrawInfo();

	};
}



