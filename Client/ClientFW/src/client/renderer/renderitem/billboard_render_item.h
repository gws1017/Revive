#pragma once

namespace client_fw
{
	class BillboardComponent;
	class TextureBillboardComponent;
	class MaterialBillboardComponent;

	template<class VertexType>
	class UploadPrimitive;
	class BillboardVertex;

	class BillboardRenderItem
	{
	public:
		BillboardRenderItem();
		virtual ~BillboardRenderItem();

		void Initialize(ID3D12Device* device);
		void Shutdown();

		virtual void Update(ID3D12Device* device) = 0;
		virtual void UpdateFrameResource(ID3D12Device* device) = 0;
		virtual void Draw(ID3D12GraphicsCommandList* command_list,
			std::function<void()>&& draw_function, std::function<void()>&& fix_up_draw_function) = 0;

		virtual void RegisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) {}
		virtual void UnregisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) {}

	protected:
		bool m_is_need_resource_create = false;

		UINT m_num_of_billboard_data = 1;
		std::vector<BillboardVertex> m_vertices;
	};
	
	class TextureBillboardRenderItem final : public BillboardRenderItem
	{
	public:
		TextureBillboardRenderItem();
		virtual ~TextureBillboardRenderItem();

		virtual void Update(ID3D12Device* device) override;
		virtual void UpdateFrameResource(ID3D12Device* device) override;
		virtual void Draw(ID3D12GraphicsCommandList* command_list,
			std::function<void()>&& draw_function, std::function<void()>&& fix_up_draw_function) override;

		virtual void RegisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) override;
		virtual void UnregisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) override;

	private:
		std::vector<SPtr<TextureBillboardComponent>> m_billboard_components;
	};

	class MaterialBillboardRenderItem final : public BillboardRenderItem
	{
	public:
		MaterialBillboardRenderItem();
		virtual ~MaterialBillboardRenderItem();

		virtual void Update(ID3D12Device* device) override;
		virtual void UpdateFrameResource(ID3D12Device* device) override;
		virtual void Draw(ID3D12GraphicsCommandList* command_list,
			std::function<void()>&& draw_function, std::function<void()>&& fix_up_draw_function);

		virtual void RegisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) override;
		virtual void UnregisterBillboardComponent(const SPtr<BillboardComponent>& bb_comp) override;

	private:
		std::vector<SPtr<MaterialBillboardComponent>> m_billboard_components;
	};
}



