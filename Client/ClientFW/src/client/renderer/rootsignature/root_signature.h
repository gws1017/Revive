#pragma once

namespace client_fw
{
	class RootSignature
	{
	public:
		RootSignature() = default;

		bool Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);
		virtual void Shutdown() {}
		virtual void Draw(ID3D12GraphicsCommandList* command_list) = 0;
		virtual bool CreateRootSignature(ID3D12Device* device) = 0;
		virtual void CreateResources(ID3D12Device* device) {}
		virtual void UpdateResources() {}

	protected:
		ComPtr<ID3D12RootSignature> m_root_signature;

	public:
		ID3D12RootSignature* GetRootSignature() const { return m_root_signature.Get(); }
	};
}



