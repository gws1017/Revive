#include "stdafx.h"
#include "client/input/input.h"
#include "client/input/input_event_system.h"
#include "client/input/input_manager.h"
#include "client/input/input_event_manager.h"

namespace client_fw
{
	bool Input::IsKeyHoldDown(EKey key)
	{
		return s_input_event_system->GetInputManager()->IsKeyHoldDown(ToUnderlying(key));
	}

	bool Input::IsKeyHoldDown(EAdditionalKey key)
	{
		return s_input_event_system->GetInputManager()->IsKeyHoldDown(ToUnderlying(key));
	}

	bool Input::IsKeyPressed(EKey key)
	{
		return s_input_event_system->GetInputManager()->IsKeyPressed(ToUnderlying(key));
	}

	bool Input::IsKeyReleased(EKey key)
	{
		return s_input_event_system->GetInputManager()->IsKeyReleased(ToUnderlying(key));
	}

	bool Input::IsNotKeyHoldDown(EKey key)
	{
		return s_input_event_system->GetInputManager()->IsNotKeyHoldDown(ToUnderlying(key));
	}

	const IVec2& Input::GetMousePosition()
	{
		return s_input_event_system->GetInputManager()->GetMousePosition();
	}

	const IVec2 Input::GetRelativeMousePosition()
	{
		return s_input_event_system->GetInputManager()->GetRelativeMoustPosition();
	}

	void Input::ConsumeKey(EKey key)
	{
		s_input_event_system->GetInputManager()->ConsumeKey(ToUnderlying(key));
	}

	bool Input::IsConsumedKey(EKey key)
	{
		return s_input_event_system->GetInputManager()->IsConsumedKey(ToUnderlying(key));
	}

	void Input::SetHideCursor(bool hide)
	{
		if (hide != IsHideCursor())
			s_input_event_system->GetInputManager()->SetHideCursor(hide);
	}

	bool Input::IsHideCursor()
	{
		return s_input_event_system->GetInputManager()->IsHideCursor();
	}

	void Input::SetClipCursor(bool clip)
	{
		if (clip != IsClipCursor())
			s_input_event_system->GetInputManager()->SetClipCursor(clip);
	}

	bool Input::IsClipCursor()
	{
		return s_input_event_system->GetInputManager()->IsClipCursor();
	}

	void Input::RegisterPressedEvent(std::string_view name, std::vector<EventKeyInfo>&& keys,
		const std::function<void()>& func, bool consumption, EInputOwnerType type)
	{
		s_input_event_system->GetInputEventManager()->RegisterEvent(CreateUPtr<PressedEventInfo>(name, consumption, std::move(keys), func), type);
	}
}