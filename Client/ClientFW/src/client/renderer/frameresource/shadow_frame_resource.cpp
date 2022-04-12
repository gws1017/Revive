#include "stdafx.h"
#include "client/renderer/frameresource/shadow_frame_resource.h"
#include "client/util/upload_buffer.h"

namespace client_fw
{
	ShadowFrameResource::ShadowFrameResource()
	{
		m_shadow_texture_data = CreateUPtr<UploadBuffer<RSShadowTextureData>>();
	}

	ShadowFrameResource::~ShadowFrameResource()
	{
	}

	bool ShadowFrameResource::Initialize(ID3D12Device* device)
	{
		m_shadow_texture_data->CreateResource(device, m_size_of_shadow_texture);
		return true;
	}

	void ShadowFrameResource::Shutdown()
	{
		m_shadow_texture_data->Shutdown();
	}
}
