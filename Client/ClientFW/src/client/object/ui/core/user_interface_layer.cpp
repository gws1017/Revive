#include "stdafx.h"
#include "client/object/ui/core/user_interface_layer.h"
#include "client/object/ui/core/user_interface.h"

namespace client_fw
{
	UserInterfaceLayer::UserInterfaceLayer(const std::string& name)
		: m_name(name), m_layer_state(eUILayerState::kActive)
	{
	}

	bool UserInterfaceLayer::InitlaizeUILayer()
	{
		bool ret = false;
		ret = Initialize();
		return  ret;
	}

	void UserInterfaceLayer::ShutdownUILayer()
	{
		Shutdown();

		for (const auto& ui : m_user_interfaces)
			ui->Shutdown();
	}

	void UserInterfaceLayer::UpdateUILayer(float delta_time)
	{
		Update(delta_time);
	}

	bool UserInterfaceLayer::RegisterUserInterface(const SPtr<UserInterface>& ui)
	{
		if (ui->Initialize())
		{
			m_num_of_visible_texture += ui->GetNumOfVisibleTexture();
			m_user_interfaces.push_back(ui);
			return true;
		}
		else
		{
			LOG_ERROR("Could not initialize user interface : {0}", ui->GetName());
			ui->Shutdown();
			return false;
		}
	}
}
