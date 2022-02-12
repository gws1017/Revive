#pragma once
#include "client/renderer/shader/core/shader.h"

namespace client_fw
{
	class GraphicsRenderLevel;
	class MeshComponent;
	class MeshRenderItem;

	class GraphicsShader : public Shader
	{
	public:
		GraphicsShader(const std::string& name);
		virtual ~GraphicsShader() = default;
		
		virtual void UpdateRenderItem(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) = 0;
		virtual void DrawRenderItem(ID3D12GraphicsCommandList* command_list) const = 0;

		virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** shader_blob, eRenderLevelType level_type, int pso_index)  const= 0;
		virtual D3D12_SHADER_BYTECODE CreateHullShader(ID3DBlob** shader_blob, eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_SHADER_BYTECODE CreateDomainShader(ID3DBlob** shader_blob, eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_SHADER_BYTECODE CreateGeometryShader(ID3DBlob** shader_blob, eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** shader_blob, eRenderLevelType level_type, int pso_index) const;

		virtual std::vector<D3D12_INPUT_ELEMENT_DESC> CreateInputLayout(eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_RASTERIZER_DESC CreateRasterizerState(eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_BLEND_DESC CreateBlendState(eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState(eRenderLevelType level_type, int pso_index) const;
		virtual D3D12_STREAM_OUTPUT_DESC CreateStreamOutputState(eRenderLevelType level_type, int pso_index) const;

		virtual D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(eRenderLevelType level_type, int pso_index) const;

		virtual bool CreatePipelineStates(ID3D12Device* device, const SPtr<GraphicsRenderLevel>& render_level) = 0;

	protected:
		bool CreatePipelineState(ID3D12Device* device, const SPtr<GraphicsRenderLevel>& render_level, int num_of_pso);

	private:
		virtual D3D12_SHADER_BYTECODE CreateShader(ID3DBlob** shader_blob) const;

	public:
		virtual bool RegisterMeshComponent(ID3D12Device* device, const SPtr<MeshComponent>& mesh_comp) = 0;
		virtual void UnregisterMeshComponent(const SPtr<MeshComponent>& mesh_comp) = 0;
		//virtual bool RegisterBillboardComponent(ID3D12Device* device, const SPtr<BillboardComponent>& comp) = 0;
	};

	class MeshShader : public GraphicsShader
	{
	public:
		MeshShader(const std::string& name);
		virtual ~MeshShader() = default;

		virtual void Initialize(ID3D12Device* device) override;
		virtual void Shutdown() override;

	protected:
		virtual void UpdateRenderItem(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) override final;
		virtual void DrawRenderItem(ID3D12GraphicsCommandList* command_list) const override final;

		virtual bool RegisterMeshComponent(ID3D12Device* device, const SPtr<MeshComponent>& mesh_comp) override final;
		virtual void UnregisterMeshComponent(const SPtr<MeshComponent>& mesh_comp) override final;

	private:
		std::vector<SPtr<MeshRenderItem>> m_render_items;
		std::unordered_map<std::string, SPtr<MeshRenderItem>> m_render_items_map;
	};
}


