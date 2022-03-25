#include "stdafx.h"
#include "client/renderer/rootsignature/graphics_super_root_signature.h"
#include "client/util/d3d_util.h"

#include "client/object/actor/core/actor.h"
#include "client/object/component/util/camera_component.h"


namespace client_fw
{
	GraphicsSuperRootSignature::GraphicsSuperRootSignature()
	{
	}

	GraphicsSuperRootSignature::~GraphicsSuperRootSignature()
	{
		
	}

	void GraphicsSuperRootSignature::Shutdown()
	{
	}

	void GraphicsSuperRootSignature::Draw(ID3D12GraphicsCommandList* command_list) const
	{
		command_list->SetGraphicsRootSignature(m_root_signature.Get());
	}

	bool GraphicsSuperRootSignature::CreateRootSignature(ID3D12Device* device)
	{
		std::array<CD3DX12_DESCRIPTOR_RANGE, 1> descriptor_range;
		descriptor_range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, -1, 0, 1);

		std::array<CD3DX12_ROOT_PARAMETER, 6> root_parameters;
		root_parameters[0].InitAsConstantBufferView(0, 0);
		root_parameters[1].InitAsShaderResourceView(0, 0);
		root_parameters[2].InitAsConstantBufferView(1, 0);
		root_parameters[3].InitAsShaderResourceView(1, 0);
		root_parameters[4].InitAsDescriptorTable(1, &descriptor_range[0]);
		root_parameters[5].InitAsShaderResourceView(2, 0); //bone_transform

		std::array<CD3DX12_STATIC_SAMPLER_DESC, 1> static_samplers;
		static_samplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc(static_cast<UINT>(root_parameters.size()), root_parameters.data(),
			static_cast<UINT>(static_samplers.size()), static_samplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature_blob = nullptr;
		ComPtr<ID3DBlob> error_blob = nullptr;
	
		if (FAILED(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
			signature_blob.GetAddressOf(), error_blob.GetAddressOf())))
		{
			return false;
		}

		if (error_blob != nullptr)
		{
			OutputDebugStringA(static_cast<char*>(error_blob->GetBufferPointer()));
		}

		return SUCCEEDED(device->CreateRootSignature(0, signature_blob->GetBufferPointer(),
			signature_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));

	}
}
