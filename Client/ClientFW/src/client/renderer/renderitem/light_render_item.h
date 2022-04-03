#pragma once

namespace client_fw
{
	struct RSLocalLightInstanceData;
	class LightComponent;

	class LightRenderItem
	{
	public:
		LightRenderItem(const std::string& owner_shader_name);
		virtual ~LightRenderItem();

		void Initialize(ID3D12Device* device);
		void Shutdown();

		virtual void Update(ID3D12Device* device);
		virtual void UpdateFrameResource(ID3D12Device* device);
		virtual void Draw(ID3D12GraphicsCommandList* command_list, std::function<void()>&& draw_function) const = 0;

		virtual void RegisterLightComponent(const SPtr<LightComponent>& light_comp);
		virtual void UnregisterLightComponent(const SPtr<LightComponent>& light_comp);

	protected:
		std::string m_owner_shader_name;
		std::vector<SPtr<LightComponent>> m_light_components;

		std::vector<RSLocalLightInstanceData> m_local_light_instance_data;
	};

	class PointLightRenderItem : public LightRenderItem
	{
	public:
		PointLightRenderItem(const std::string& owner_shader_name);
		virtual ~PointLightRenderItem() = default;

		virtual void Draw(ID3D12GraphicsCommandList* command_list, std::function<void()>&& draw_function) const override;
	};



}
