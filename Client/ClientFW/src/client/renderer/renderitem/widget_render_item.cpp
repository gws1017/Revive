#include "stdafx.h"
#include "client/renderer/renderitem/widget_render_item.h"
#include "client/renderer/frameresource/core/frame_resource_manager.h"
#include "client/renderer/frameresource/core/frame_resource.h"
#include "client/renderer/frameresource/widget_frame_resource.h"
#include "client/asset/primitive/primitive.h"
#include "client/asset/texture/texture.h"
#include "client/object/component/render/widget_component.h"
#include "client/object/ui/core/user_interface_layer.h"
#include "client/object/ui/core/user_interface.h"
#include "client/object/actor/core/actor.h"

namespace client_fw
{
	WidgetRenderItem::WidgetRenderItem()
	{
	}

	WidgetRenderItem::~WidgetRenderItem()
	{
	}

	void WidgetRenderItem::Initialize(ID3D12Device* device)
	{
	}

	void WidgetRenderItem::Shutdown()
	{
	}

	void WidgetRenderItem::Update(ID3D12Device* device)
	{
		UpdateWorldWidgets(device);
		UpdatePivotWidgets(device);		
	}

	void WidgetRenderItem::UpdateWorldWidgets(ID3D12Device* device)
	{
		WorldWidgetDrawInfo info;
		info.start_index = static_cast<UINT>(m_world_widget_vertices.size());

		for (const auto& widget : m_world_widget_components)
		{
			Vec3 position = widget->GetWorldPosition();
			const Vec3& right = widget->GetWorldRight();
			const Vec3& up = widget->GetWorldUp();

			const Vec2& widget_size = widget->GetSize();
			const Vec2& move_pos = widget->GetPivot() * widget_size;
			position += ((up * move_pos.y) - (right * move_pos.x));

			if (widget->IsVisible())
			{
				for (const auto& ui : widget->GetUserInterfaceLayer()->GetUserInterfaces())
				{
					const Vec2& lt = ui->GetPosition();
					if (ui->IsVisible() &&
						((lt.x < 0) || (lt.y < 0) || (lt.x > widget_size.x) || (lt.y > widget_size.y)) == false)
					{
						for (const auto& ui_texture : ui->GetVisibleTextures())
						{
							if (ui_texture == nullptr)
								LOG_WARN("Could not find ui texture : {0}", ui->GetName());
							else
							{
								Vec3 new_position = position + right * lt.x + up * lt.y;

								INT resource_index = -1;

								if (ui_texture->GetTexture() != nullptr)
									resource_index = ui_texture->GetTexture()->GetResourceIndex();

								m_world_widget_vertices.emplace_back(WorldWidgetVertex(new_position, ui_texture->GetSize(),
									resource_index,	ui_texture->GetBrushColor(), ui_texture->GetCoordinate(),
									ui_texture->GetTilling(), right, up));
							}
						}
					}
				}

				widget->SetVisiblity(false);
			}
		}

		info.num_of_draw_data = static_cast<UINT>(m_world_widget_vertices.size() - info.start_index);
	
		const auto& widget_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetWidgetFrameResource();
		widget_resource->AddWorldWidgetDrawInfo(std::move(info));
	}

	void WidgetRenderItem::UpdatePivotWidgets(ID3D12Device* device)
	{
		std::vector<PivotWidgetVertex> vertices;
		std::vector<PivotWidgetVertex> fix_up_vertices;

		for (const auto& widget : m_widget_components)
		{
			Vec3 position = widget->GetWorldPosition();
			const Vec3& right = widget->GetWorldRight();
			const Vec3& up = widget->GetWorldUp();

			const Vec2& widget_size = widget->GetSize();
			const Vec2& pivot = widget->GetPivot() * widget_size;

			if (widget->IsVisible())
			{
				for (const auto& ui : widget->GetUserInterfaceLayer()->GetUserInterfaces())
				{
					const Vec2& lt = ui->GetPosition();
					if (ui->IsVisible() &&
						((lt.x < 0) || (lt.y < 0) || (lt.x > widget_size.x) || (lt.y > widget_size.y)) == false)
					{
						for (const auto& ui_texture : ui->GetVisibleTextures())
						{
							if (ui_texture == nullptr)
								LOG_WARN("Could not find ui texture : {0}", ui->GetName());
							else
							{
								Vec3 new_position = position + right * lt.x + up * lt.y;

								INT resource_index = -1;

								if (ui_texture->GetTexture() != nullptr)
									resource_index = ui_texture->GetTexture()->GetResourceIndex();

								PivotWidgetVertex vertex(PivotWidgetVertex(new_position, ui_texture->GetSize(), resource_index,
									ui_texture->GetBrushColor(), ui_texture->GetCoordinate(), ui_texture->GetTilling(), pivot));

								if (widget->GetWidgetSpaceType() == eWidgetSpaceType::kBillboard)
									vertices.emplace_back(std::move(vertex));
								else if (widget->GetWidgetSpaceType() == eWidgetSpaceType::kFixUpBillboard)
									fix_up_vertices.emplace_back(std::move(vertex));
							}
						}
					}
				}

				widget->SetVisiblity(false);
			}
		}

		const auto& widget_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetWidgetFrameResource();

		PivotWidgetDrawInfo info;
		info.billboard_start_index = static_cast<UINT>(m_pivot_widget_vertices.size());
		info.num_of_draw_billboard_data = static_cast<UINT>(vertices.size());
		std::move(vertices.begin(), vertices.end(), std::back_inserter(m_pivot_widget_vertices));
		info.fix_up_start_index = static_cast<UINT>(m_pivot_widget_vertices.size());
		info.num_of_draw_fix_up_data = static_cast<UINT>(fix_up_vertices.size());
		std::move(fix_up_vertices.begin(), fix_up_vertices.end(), std::back_inserter(m_pivot_widget_vertices));

		widget_resource->AddPivotWidgetDrawInfo(std::move(info));
	}

	void WidgetRenderItem::UpdateFrameResource(ID3D12Device* device)
	{
		const auto& widget_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetWidgetFrameResource();

		UINT new_size = static_cast<UINT>(m_world_widget_vertices.size());
		if (new_size > 0)
		{
			bool is_need_resource_create = false;

			while (m_num_of_world_widget_ui_data <= new_size)
			{
				m_num_of_world_widget_ui_data = static_cast<UINT>(roundf(static_cast<float>(m_num_of_world_widget_ui_data) * 1.5f));
				is_need_resource_create = true;
			}

			if (is_need_resource_create)
				widget_resource->GetWorldWidgetPrimitive()->Update(device, m_num_of_world_widget_ui_data);

			widget_resource->GetWorldWidgetPrimitive()->UpdateVertices(m_world_widget_vertices);
			m_world_widget_vertices.clear();
		}

		new_size = static_cast<UINT>(m_pivot_widget_vertices.size());
		if (new_size > 0)
		{
			bool is_need_resource_create = false;

			while (m_num_of_pivot_widget_ui_data <= new_size)
			{
				m_num_of_pivot_widget_ui_data = static_cast<UINT>(roundf(static_cast<float>(m_num_of_pivot_widget_ui_data) * 1.5f));
				is_need_resource_create = true;
			}

			if (is_need_resource_create)
				widget_resource->GetPivotWidgetPrimitive()->Update(device, m_num_of_pivot_widget_ui_data);

			widget_resource->GetPivotWidgetPrimitive()->UpdateVertices(m_pivot_widget_vertices);
			m_pivot_widget_vertices.clear();
		}
	}

	void WidgetRenderItem::Draw(ID3D12GraphicsCommandList* command_list,
		std::function<void()>&& world_function, std::function<void()>&& billboard_function,
		std::function<void()>&& fix_up_function)
	{
		const auto& widget_resource = FrameResourceManager::GetManager().GetCurrentFrameResource()->GetWidgetFrameResource();
		WorldWidgetDrawInfo world_info = widget_resource->GetWorldWidgetDrawInfo();

		if (world_info.num_of_draw_data > 0)
		{
			world_function();
			widget_resource->GetWorldWidgetPrimitive()->PreDraw(command_list);
			widget_resource->GetWorldWidgetPrimitive()->Draw(command_list, world_info.num_of_draw_data, world_info.start_index);
		}

		PivotWidgetDrawInfo pivot_info = widget_resource->GetPivotWidgetDrawInfo();

		if (pivot_info.num_of_draw_billboard_data + pivot_info.num_of_draw_fix_up_data > 0)
		{
			widget_resource->GetPivotWidgetPrimitive()->PreDraw(command_list);
			if (pivot_info.num_of_draw_billboard_data > 0)
			{
				billboard_function();
				widget_resource->GetPivotWidgetPrimitive()->Draw(command_list, 
					pivot_info.num_of_draw_billboard_data, pivot_info.billboard_start_index);
			}
			if (pivot_info.num_of_draw_fix_up_data > 0)
			{
				fix_up_function();
				widget_resource->GetWorldWidgetPrimitive()->Draw(command_list,
					pivot_info.num_of_draw_fix_up_data, pivot_info.fix_up_start_index);
			}
		}
	}

	void WidgetRenderItem::RegisterWidgetComponent(const SPtr<WidgetComponent>& widget_comp)
	{
		const auto& ui_layer = widget_comp->GetUserInterfaceLayer();
		if (ui_layer != nullptr)
		{
			UINT num_of_data = ui_layer->GetNumOfVisibleTexture();
			if (widget_comp->GetWidgetSpaceType() == eWidgetSpaceType::kWorld)
			{
				widget_comp->SetRenderItemIndex(static_cast<UINT>(m_world_widget_components.size()));
				m_world_widget_components.push_back(widget_comp);
			}
			else
			{
				widget_comp->SetRenderItemIndex(static_cast<UINT>(m_widget_components.size()));
				m_widget_components.push_back(widget_comp);
			}
		}
	}

	void WidgetRenderItem::UnregisterWidgetComponent(const SPtr<WidgetComponent>& widget_comp)
	{
		const auto& ui_layer = widget_comp->GetUserInterfaceLayer();
		if (ui_layer != nullptr)
		{
			UINT index = widget_comp->GetRenderItemIndex();
			UINT num_of_data = ui_layer->GetNumOfVisibleTexture();

			if (widget_comp->GetWidgetSpaceType() == eWidgetSpaceType::kWorld)
			{
				std::iter_swap(m_world_widget_components.begin() + index, m_world_widget_components.end() - 1);
				m_world_widget_components[index]->SetRenderItemIndex(index);
				m_world_widget_components.pop_back();
			}
			else
			{
				std::iter_swap(m_widget_components.begin() + index, m_widget_components.end() - 1);
				m_widget_components[index]->SetRenderItemIndex(index);
				m_widget_components.pop_back();
			}
		}
	}
}
