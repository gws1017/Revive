#pragma once

namespace client_fw
{
	class BillboardFrameResource;

	class FrameResource
	{
	public:
		FrameResource();
		~FrameResource();

		bool Initialize(ID3D12Device* device);
		void Shutdown();

	private:
		ComPtr<ID3D12CommandAllocator> m_command_allocator;
		UINT64 m_fence = 0;

	public:
		ComPtr<ID3D12CommandAllocator> GetCommandAllocator() const { return m_command_allocator; }
		UINT64 GetFence() const { return m_fence; }
		void SetFence(UINT64 value) { m_fence = value; }

	private:
		UPtr<BillboardFrameResource> m_billboard_frame_resource;

	public:
		const UPtr<BillboardFrameResource>& GetBillboardFrameResource() const { return m_billboard_frame_resource; }


	};
}



