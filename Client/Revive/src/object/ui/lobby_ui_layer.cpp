#include <include/client_core.h>
#include <client/object/ui/text_ui.h>
#include <client/object/ui/button_ui.h>
#include <client/object/ui/image_ui.h>
#include <client/object/ui/text_box_ui.h>
#include <client/renderer/core/render.h>
#include <client/event/packetevent/packet_helper.h>
#include <client/object/level/core/level_manager.h>
#include <client/object/level/core/level_loader.h>

#include "revive_server/message/message_event_info.h"
#include "object/ui/lobby_ui_layer.h"
#include "object/level/game_play_level.h"

namespace revive
{
	LobbyUILayer::LobbyUILayer()
		: UserInterfaceLayer("lobby ui layer")
	{
		m_id_text_box = CreateSPtr<TextBoxUI>("id text box", Vec2(200.0f, 50.0f));
		m_pw_text_box = CreateSPtr<TextBoxUI>("pw text box", Vec2(200.0f, 50.0f));
		m_sign_up_button = CreateSPtr<ButtonUI>("sign up button");
		m_sign_in_button = CreateSPtr<ButtonUI>("sign in button");
		m_matching_button = CreateSPtr<ButtonUI>("matching button");
		m_num_of_user_text = CreateSPtr<TextUI>("num of user text", Vec2(300.0f, 50.0f), L"��Ī �ο� : " + std::to_wstring(m_num_of_user));
		m_user_change_button = CreateSPtr<ButtonUI>("user change button");

		m_invalid_id_pw_image = CreateSPtr<ImageUI>("invalid id pw image", Vec2(400.0f, 400.0f));
		m_invalid_id_pw_ok_button = CreateSPtr<ButtonUI>("invalid id pw ok button");
		m_already_logged_in_image = CreateSPtr<ImageUI>("already logged in image", Vec2(400.0f, 400.0f));
		m_already_logged_in_ok_button = CreateSPtr<ButtonUI>("already logged in ok button");
		m_login_succeed_image = CreateSPtr<ImageUI>("login succeed image", Vec2(400.0f, 400.0f));
		m_login_succeed_ok_button = CreateSPtr<ButtonUI>("login succeed ok button");

		m_not_login_image = CreateSPtr<ImageUI>("no login image", Vec2(400.0f, 400.0f));
		m_not_login_ok_button = CreateSPtr<ButtonUI>("not login ok button");
		m_matching_image = CreateSPtr<ImageUI>("matching image", Vec2(400.0f, 400.0f));
		m_matching_cancel_button = CreateSPtr<ButtonUI>("matching ok button");
		m_matching_time_text = CreateSPtr<TextUI>("matching time text", Vec2(350.0f, 50.0f), L"��Ī �ð� : 0 : 0");
		m_matching_success_image = CreateSPtr<ImageUI>("matching success iamge", Vec2(400.0f, 400.0f));
		m_game_start_button = CreateSPtr<ButtonUI>("game start button");
	}

	bool LobbyUILayer::Initialize()
	{
		Vec2 window_size = Render::GetWindowSize();

		bool ret = GenerateMatchingUI(window_size);
		ret &= GenerateSignInMenuUI(window_size);
		ret &= GenerateMatchingMenuUI(window_size);

		return ret;
	}

	void LobbyUILayer::Update(float delta_time)
	{
		if (m_is_wait_matching)
		{
			m_matching_time += delta_time;
			m_matching_time_text->SetText(L"��Ī �ð� [ " + std::to_wstring(static_cast<UINT>(m_matching_time) / 60) +
				L" : " + std::to_wstring(static_cast<UINT>(m_matching_time) % 60) + L" ]");
		}
	}

	bool LobbyUILayer::GenerateMatchingUI(const Vec2& window_size)
	{
		bool ret = true;
		
		m_id_text_box->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
		m_id_text_box->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_id_text_box->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_id_text_box->SetFontSize(20);
		m_id_text_box->SetPosition(window_size * 0.5f - Vec2(0.0f, 160.0f));
		m_id_text_box->OnCommitted([this]() {
			m_id = m_id_text_box->GetText();
			});
		ret &= RegisterUserInterface(m_id_text_box);

		m_pw_text_box->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
		m_pw_text_box->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_pw_text_box->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_pw_text_box->SetFontSize(20);
		m_pw_text_box->SetPosition(window_size * 0.5f - Vec2(0.0f, 100.0f));
		m_pw_text_box->OnCommitted([this]() {
			m_pw = m_pw_text_box->GetText();
			});
		ret &= RegisterUserInterface(m_pw_text_box);


		m_sign_up_button->SetNormalTexture("Contents/ui/sign_up_normal.dds");
		m_sign_up_button->SetHoveredTexture("Contents/ui/sign_up_hovered.dds");
		m_sign_up_button->SetPressedTexture("Contents/ui/sign_up_pressed.dds");
		m_sign_up_button->SetPosition(window_size * 0.5f);
		m_sign_up_button->SetSize(Vec2(300.0f, 120.0f));
		m_sign_up_button->OnClicked([this]() {
			PacketHelper::RegisterPacketEventToServer(CreateSPtr<SignUpMessageEventInfo>(HashCode("send sign up"), m_id.data(), m_pw.data()));
			});
		ret &= RegisterUserInterface(m_sign_up_button);

		m_sign_in_button->SetNormalTexture("Contents/ui/sign_in_normal.dds");
		m_sign_in_button->SetHoveredTexture("Contents/ui/sign_in_hovered.dds");
		m_sign_in_button->SetPressedTexture("Contents/ui/sign_in_pressed.dds");
		m_sign_in_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 130.0f));
		m_sign_in_button->SetSize(Vec2(300.0f, 120.0f));
		m_sign_in_button->OnClicked([this]() {
			PacketHelper::RegisterPacketEventToServer(CreateSPtr<SignInMessageEventInfo>(HashCode("send sign in"), m_id.data(), m_pw.data()));
			});
		ret &= RegisterUserInterface(m_sign_in_button);


		m_matching_button->SetNormalTexture("Contents/ui/matching_normal.dds");
		m_matching_button->SetHoveredTexture("Contents/ui/matching_hovered.dds");
		m_matching_button->SetPressedTexture("Contents/ui/matching_pressed.dds");
		m_matching_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 260.0f));
		m_matching_button->SetSize(Vec2(300.0f, 120.0f));
		m_matching_button->OnClicked([this]() {
			if (m_succeeded_login)
			{
				PacketHelper::RegisterPacketEventToServer(CreateSPtr<MatchingMessageEventInfo>(HashCode("send sign matching"), m_num_of_user));
				m_is_wait_matching = true;
				EnablePopUpState(eLobbyPopupMenu::kMatching);
				m_matching_time = 0.0f;
			}
			else
			{
				EnablePopUpState(eLobbyPopupMenu::kNotLogin);
			}});
		ret &= RegisterUserInterface(m_matching_button);

		m_num_of_user_text->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
		m_num_of_user_text->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
		m_num_of_user_text->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_num_of_user_text->SetFontSize(30);
		m_num_of_user_text->SetPosition(window_size * 0.5f + Vec2(0.0f, 370.0f));
		ret &= RegisterUserInterface(m_num_of_user_text);

		m_user_change_button->SetNormalTexture("Contents/ui/user_num_button.dds");
		m_user_change_button->SetHoveredTexture("Contents/ui/user_num_button_hovered.dds");
		m_user_change_button->SetPressedTexture("Contents/ui/user_num_button_pressed.dds");
		m_user_change_button->SetPosition(window_size * 0.5f + Vec2(100.0f, 370.0f));
		m_user_change_button->SetSize(Vec2(100.0f, 100.0f));
		m_user_change_button->OnClicked([this]() {
			++m_num_of_user;
			if (m_num_of_user == 4)
				m_num_of_user = 2;
			m_num_of_user_text->SetText(L"��Ī �ο� : " + std::to_wstring(m_num_of_user));
			});
		ret &= RegisterUserInterface(m_user_change_button);

		return ret;
	}

	bool LobbyUILayer::GenerateSignInMenuUI(const Vec2& window_size)
	{
		bool ret = true;

		m_invalid_id_pw_image->SetTexture("Contents/ui/invalid_id_pw_menu.dds");
		m_invalid_id_pw_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_invalid_id_pw_image);

		m_invalid_id_pw_ok_button->SetNormalTexture("Contents/ui/ok_normal.dds");
		m_invalid_id_pw_ok_button->SetHoveredTexture("Contents/ui/ok_hovered.dds");
		m_invalid_id_pw_ok_button->SetPressedTexture("Contents/ui/ok_pressed.dds");
		m_invalid_id_pw_ok_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_invalid_id_pw_ok_button->SetSize(Vec2(300.0f, 120.0f));
		m_invalid_id_pw_ok_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kInvalidIDPW);
			});
		DisablePopUpState(eLobbyPopupMenu::kInvalidIDPW);
		ret &= RegisterUserInterface(m_invalid_id_pw_ok_button);

		m_already_logged_in_image->SetTexture("Contents/ui/already_logged_in_menu.dds");
		m_already_logged_in_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_already_logged_in_image);

		m_already_logged_in_ok_button->SetNormalTexture("Contents/ui/ok_normal.dds");
		m_already_logged_in_ok_button->SetHoveredTexture("Contents/ui/ok_hovered.dds");
		m_already_logged_in_ok_button->SetPressedTexture("Contents/ui/ok_pressed.dds");
		m_already_logged_in_ok_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_already_logged_in_ok_button->SetSize(Vec2(300.0f, 120.0f));
		m_already_logged_in_ok_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kAlreadyLogin);
			});
		DisablePopUpState(eLobbyPopupMenu::kAlreadyLogin);
		ret &= RegisterUserInterface(m_already_logged_in_ok_button);

		m_login_succeed_image->SetTexture("Contents/ui/log_in_succeed_menu.dds");
		m_login_succeed_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_login_succeed_image);

		m_login_succeed_ok_button->SetNormalTexture("Contents/ui/ok_normal.dds");
		m_login_succeed_ok_button->SetHoveredTexture("Contents/ui/ok_hovered.dds");
		m_login_succeed_ok_button->SetPressedTexture("Contents/ui/ok_pressed.dds");
		m_login_succeed_ok_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_login_succeed_ok_button->SetSize(Vec2(300.0f, 120.0f));
		m_login_succeed_ok_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kLoginSuccess);
			});
		DisablePopUpState(eLobbyPopupMenu::kLoginSuccess);
		ret &= RegisterUserInterface(m_login_succeed_ok_button);
		

		return ret;
	}

	bool LobbyUILayer::GenerateMatchingMenuUI(const Vec2& window_size)
	{
		bool ret = true;

		m_not_login_image->SetTexture("Contents/ui/log_in_before_matching_menu.dds");
		m_not_login_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_not_login_image);

		m_not_login_ok_button->SetNormalTexture("Contents/ui/ok_normal.dds");
		m_not_login_ok_button->SetHoveredTexture("Contents/ui/ok_hovered.dds");
		m_not_login_ok_button->SetPressedTexture("Contents/ui/ok_pressed.dds");
		m_not_login_ok_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_not_login_ok_button->SetSize(Vec2(300.0f, 120.0f));
		m_not_login_ok_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kNotLogin);
			});
		DisablePopUpState(eLobbyPopupMenu::kNotLogin);
		ret &= RegisterUserInterface(m_not_login_ok_button);

		m_matching_image->SetTexture("Contents/ui/mathcing_menu.dds");
		m_matching_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_matching_image);

		m_matching_time_text->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD);
		m_matching_time_text->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		m_matching_time_text->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		m_matching_time_text->SetFontSize(40);
		m_matching_time_text->SetColor(Vec4(239.0f / 255.0f, 71.0f / 255.0f, 84.0f / 255.0f, 1.0f));
		m_matching_time_text->SetPosition(window_size * 0.5f - Vec2(0.0f, 10.0f));
		ret &= RegisterUserInterface(m_matching_time_text);

		m_matching_cancel_button->SetNormalTexture("Contents/ui/cancel_normal.dds");
		m_matching_cancel_button->SetHoveredTexture("Contents/ui/cancel_hovered.dds");
		m_matching_cancel_button->SetPressedTexture("Contents/ui/cancel_pressed.dds");
		m_matching_cancel_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_matching_cancel_button->SetSize(Vec2(300.0f, 120.0f));
		m_matching_cancel_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kMatching);
			m_is_wait_matching = false;
			});
		DisablePopUpState(eLobbyPopupMenu::kMatching);
		ret &= RegisterUserInterface(m_matching_cancel_button);


		m_matching_success_image->SetTexture("Contents/ui/matching_success_menu.dds");
		m_matching_success_image->SetPosition(window_size * 0.5f);
		ret &= RegisterUserInterface(m_matching_success_image);

		m_game_start_button->SetNormalTexture("Contents/ui/game_start_normal.dds");
		m_game_start_button->SetHoveredTexture("Contents/ui/game_start_hovered.dds");
		m_game_start_button->SetPressedTexture("Contents/ui/game_start_pressed.dds");
		m_game_start_button->SetPosition(window_size * 0.5f + Vec2(0.0f, 120.0f));
		m_game_start_button->SetSize(Vec2(300.0f, 120.0f));
		m_game_start_button->OnClicked([this]() {
			DisablePopUpState(eLobbyPopupMenu::kGameStart);
			LevelManager::GetLevelManager().OpenLevel(CreateSPtr<GamePlayLevel>(), nullptr);
			});
		DisablePopUpState(eLobbyPopupMenu::kGameStart);
		ret &= RegisterUserInterface(m_game_start_button);

		return ret;
	}


	void LobbyUILayer::SetPopUpState(eLobbyPopupMenu state, bool is_pop_up)
	{
		m_id_text_box->SetActivate(!is_pop_up);
		m_pw_text_box->SetActivate(!is_pop_up);
		m_sign_up_button->SetActivate(!is_pop_up);
		m_sign_in_button->SetActivate(!is_pop_up);
		m_matching_button->SetActivate(!is_pop_up);
		m_num_of_user_text->SetActivate(!is_pop_up);
		m_user_change_button->SetActivate(!is_pop_up);

		switch (state)
		{
		case eLobbyPopupMenu::kInvalidIDPW:
			m_invalid_id_pw_image->SetActivate(is_pop_up);
			m_invalid_id_pw_image->SetVisible(is_pop_up);
			m_invalid_id_pw_ok_button->SetActivate(is_pop_up);
			m_invalid_id_pw_ok_button->SetVisible(is_pop_up);
			break;
		case eLobbyPopupMenu::kAlreadyLogin:
			m_already_logged_in_image->SetActivate(is_pop_up);
			m_already_logged_in_image->SetVisible(is_pop_up);
			m_already_logged_in_ok_button->SetActivate(is_pop_up);
			m_already_logged_in_ok_button->SetVisible(is_pop_up);
			break;
		case eLobbyPopupMenu::kLoginSuccess:
			m_login_succeed_image->SetActivate(is_pop_up);
			m_login_succeed_image->SetVisible(is_pop_up);
			m_login_succeed_ok_button->SetActivate(is_pop_up);
			m_login_succeed_ok_button->SetVisible(is_pop_up);
			break;
		case eLobbyPopupMenu::kNotLogin:
			m_not_login_image->SetActivate(is_pop_up);
			m_not_login_image->SetVisible(is_pop_up);
			m_not_login_ok_button->SetActivate(is_pop_up);
			m_not_login_ok_button->SetVisible(is_pop_up);
			break;
		case eLobbyPopupMenu::kMatching:
			m_matching_image->SetActivate(is_pop_up);
			m_matching_image->SetVisible(is_pop_up);
			m_matching_time_text->SetActivate(is_pop_up);
			m_matching_time_text->SetVisible(is_pop_up);
			m_matching_cancel_button->SetActivate(is_pop_up);
			m_matching_cancel_button->SetVisible(is_pop_up);
			break;
		case eLobbyPopupMenu::kGameStart:
			m_game_start_button->SetActivate(is_pop_up);
			m_game_start_button->SetVisible(is_pop_up);
			m_matching_success_image->SetActivate(is_pop_up);
			m_matching_success_image->SetVisible(is_pop_up);
			break;
		default:
			break;
		}
	}

	void LobbyUILayer::EnablePopUpState(eLobbyPopupMenu state)
	{
		SetPopUpState(state, true);
	}

	void LobbyUILayer::DisablePopUpState(eLobbyPopupMenu state)
	{
		SetPopUpState(state, false);
	}

	void LobbyUILayer::FailedLogin(eLoginFailState state)
	{
		switch (state)
		{
		case eLoginFailState::kAlreadyLogin:
			EnablePopUpState(eLobbyPopupMenu::kAlreadyLogin);
			break;
		case eLoginFailState::kInvalidIDPW:
			EnablePopUpState(eLobbyPopupMenu::kInvalidIDPW);
			break;
		default:
			break;
		}
	}

	void LobbyUILayer::SucceededLogin()
	{
		EnablePopUpState(eLobbyPopupMenu::kLoginSuccess);
		m_succeeded_login = true;
	}

	void LobbyUILayer::SucceededMatching()
	{
		EnablePopUpState(eLobbyPopupMenu::kGameStart);
		m_is_wait_matching = false;
	}
}
