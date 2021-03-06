#include "GameSettings.h"

#include <GWCA\Managers\ChatMgr.h>
#include <GWCA\Managers\PartyMgr.h>
#include <GWCA\Managers\AgentMgr.h>
#include <GWCA\Managers\StoCMgr.h>
#include <GWCA\Managers\FriendListMgr.h>

#include <logger.h>
#include "GuiUtils.h"
#include <GWToolbox.h>
#include <Timer.h>

namespace {
	void ChatEventCallback(DWORD id, DWORD type, wchar_t* info, void* unk) {
		if (type == 0x29 && GameSettings::Instance().select_with_chat_doubleclick) {
			static wchar_t last_name[64] = L"";
			static clock_t timer = TIMER_INIT();
			if (TIMER_DIFF(timer) < 500 && wcscmp(last_name, info) == 0) {
				GW::PlayerArray players = GW::Agents::GetPlayerArray();
				if (players.valid()) {
					for (unsigned i = 0; i < players.size(); ++i) {
						GW::Player& player = players[i];
						wchar_t* name = player.Name;
						if (player.AgentID > 0
							&& name != nullptr
							&& wcscmp(info, name) == 0) {
							GW::Agents::ChangeTarget(players[i].AgentID);
						}
					}
				}
			} else {
				timer = TIMER_INIT();
				wcscpy_s(last_name, info);
			}
		}
	}

	void SendChatCallback(GW::Chat::Channel chan, wchar_t msg[120]) {
		if (!GameSettings::Instance().auto_transform_url || !msg) return;
		size_t len = wcslen(msg);
		size_t max_len = 120;

		if (chan == GW::Chat::CHANNEL_WHISPER) {
			// msg == "Whisper Target Name,msg"
			size_t i;
			for (i = 0; i < len; i++)
				if (msg[i] == ',')
					break;

			if (i < len) {
				msg += i + 1;
				len -= i + 1;
				max_len -= i + 1;
			}
		}

		if (wcsncmp(msg, L"http://", 7) && wcsncmp(msg, L"https://", 8)) return;

		if (len + 5 < max_len) {
			for (int i = len; i > 0; i--)
				msg[i] = msg[i - 1];
			msg[0] = '[';
			msg[len + 1] = ';';
			msg[len + 2] = 'x';
			msg[len + 3] = 'x';
			msg[len + 4] = ']';
			msg[len + 5] = 0;
		}
	}

	void FlashWindow() {
		FLASHWINFO flashInfo = { 0 };
		flashInfo.cbSize = sizeof(FLASHWINFO);
		flashInfo.hwnd = GW::MemoryMgr::GetGWWindowHandle();
		flashInfo.dwFlags = FLASHW_TIMER | FLASHW_TRAY | FLASHW_TIMERNOFG;
		flashInfo.uCount = 0;
		flashInfo.dwTimeout = 0;
		FlashWindowEx(&flashInfo);
	}

	void WhisperCallback(const wchar_t from[20], const wchar_t msg[140]) {
		if (GameSettings::Instance().flash_window_on_pm) FlashWindow();
	}
}

void GameSettings::Initialize() {
	ToolboxModule::Initialize();
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D7C8,
		(BYTE*)"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 16));
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D320, (BYTE*)"\xEB", 1));
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D33D, (BYTE*)"\xEB", 1));

	BYTE* a = (BYTE*)"\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";
	patches.push_back(new GW::MemoryPatcher((void*)0x00669846, a, 10));
	patches.push_back(new GW::MemoryPatcher((void*)0x00669892, a, 10));
	patches.push_back(new GW::MemoryPatcher((void*)0x006698CE, a, 10));
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D5D6, a, 10));
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D622, a, 10));
	patches.push_back(new GW::MemoryPatcher((void*)0x0067D65E, a, 10));

	GW::Chat::CreateCommand(L"borderless",
		[&](std::wstring& cmd, std::wstring& a) -> bool {
		std::vector<std::wstring> args = GW::Chat::SplitString(a, ' ');
		if (args.empty()) {
			ApplyBorderless(!borderless_window);
		} else {
			std::wstring arg = GuiUtils::ToLower(args[0]);
			if (arg == L"on") {
				ApplyBorderless(true);
			} else if (arg == L"off") {
				ApplyBorderless(false);
			} else {
				Log::Error("Invalid argument '%ls', please use /borderless [|on|off]", args[0].c_str());
			}
		}
		return true;
	});

	GW::StoC::AddCallback<GW::Packet::StoC::P444>(
		[](GW::Packet::StoC::P444*) -> bool {
		if (GameSettings::Instance().flash_window_on_party_invite) FlashWindow();
		return false;
	});

	GW::StoC::AddCallback<GW::Packet::StoC::P391_InstanceLoadFile>(
		[](GW::Packet::StoC::P391_InstanceLoadFile*) -> bool {
		if (GameSettings::Instance().flash_window_on_zoning) FlashWindow();
		return false;
	});
}

void GameSettings::LoadSettings(CSimpleIni* ini) {
	ToolboxModule::LoadSettings(ini);
	borderless_window = ini->GetBoolValue(Name(), "borderlesswindow", false);
	tick_is_toggle = ini->GetBoolValue(Name(), "tick_is_toggle", true);

	show_timestamps = ini->GetBoolValue(Name(), "show_timestamps", true);
	keep_chat_history = ini->GetBoolValue(Name(), "keep_chat_history", true);
	open_template_links = ini->GetBoolValue(Name(), "openlinks", true);
	auto_transform_url = ini->GetBoolValue(Name(), "auto_url", true);
	select_with_chat_doubleclick = ini->GetBoolValue(Name(), "select_with_chat_doubleclick", true);

	flash_window_on_pm = ini->GetBoolValue(Name(), "flash_window_on_pm", true);
	flash_window_on_party_invite = ini->GetBoolValue(Name(), "flash_window_on_party_invite", true);
	flash_window_on_zoning = ini->GetBoolValue(Name(), "flash_window_on_zoning", true);

	auto_set_away = ini->GetBoolValue(Name(), "auto_set_away", false);
	auto_set_away_delay = ini->GetLongValue(Name(), "auto_set_away_delay", 10);
	auto_set_online = ini->GetBoolValue(Name(), "auto_set_online", false);

	ApplyBorderless(borderless_window);
	if (open_template_links) GW::Chat::SetOpenLinks(open_template_links);
	if (tick_is_toggle) GW::PartyMgr::SetTickToggle();
	if (select_with_chat_doubleclick) GW::Chat::SetChatEventCallback(&ChatEventCallback);
	if (auto_transform_url) GW::Chat::SetSendChatCallback(&SendChatCallback);
	if (flash_window_on_pm) GW::Chat::SetWhisperCallback(&WhisperCallback);
	
	GW::Chat::KeepChatHistory = keep_chat_history;
	GW::Chat::ShowTimestamps  = show_timestamps;
}

void GameSettings::SaveSettings(CSimpleIni* ini) {
	ToolboxModule::SaveSettings(ini);
	ini->SetBoolValue(Name(), "borderlesswindow", borderless_window);
	ini->SetBoolValue(Name(), "tick_is_toggle", tick_is_toggle);

	ini->SetBoolValue(Name(), "show_timestamps", show_timestamps);
	ini->SetBoolValue(Name(), "keep_chat_history", keep_chat_history);
	ini->SetBoolValue(Name(), "openlinks", open_template_links);
	ini->SetBoolValue(Name(), "auto_url", auto_transform_url);
	ini->SetBoolValue(Name(), "select_with_chat_doubleclick", select_with_chat_doubleclick);

	ini->SetBoolValue(Name(), "flash_window_on_pm", flash_window_on_pm);
	ini->SetBoolValue(Name(), "flash_window_on_party_invite", flash_window_on_party_invite);
	ini->SetBoolValue(Name(), "flash_window_on_zoning", flash_window_on_zoning);

	ini->SetBoolValue(Name(), "auto_set_away", auto_set_away);
	ini->SetLongValue(Name(), "auto_set_away_delay", auto_set_away_delay);
	ini->SetBoolValue(Name(), "auto_set_online", auto_set_online);
}

void GameSettings::DrawSettingInternal() {
	DrawBorderlessSetting();

	if (ImGui::Checkbox("Show chat messages timestamp.", &show_timestamps)) {
		GW::Chat::ShowTimestamps = show_timestamps;
	}

	if (ImGui::Checkbox("Keep chat history.", &keep_chat_history)) {
		GW::Chat::KeepChatHistory = keep_chat_history;
	}
	ImGui::ShowHelp("Messages in the chat do not disappear on character change.");

	if (ImGui::Checkbox("Open web links from templates", &open_template_links)) {
		GW::Chat::SetOpenLinks(open_template_links);
	}
	ImGui::ShowHelp("Clicking on template that has a URL as name will open that URL in your browser");

	if (ImGui::Checkbox("Automatically change urls into build templates.", &auto_transform_url)) {
		GW::Chat::SetSendChatCallback(&SendChatCallback);
	}
	ImGui::ShowHelp("When you write a message starting with 'http://' or 'https://', it will be converted in template format");

	if (ImGui::Checkbox("Tick is a toggle", &tick_is_toggle)) {
		if (tick_is_toggle) {
			GW::PartyMgr::SetTickToggle();
		} else {
			GW::PartyMgr::RestoreTickToggle();
		}
	}
	ImGui::ShowHelp("Ticking in party window will work as a toggle instead of opening the menu");

	if (ImGui::Checkbox("Target with double-click on message author", &select_with_chat_doubleclick)) {
		GW::Chat::SetChatEventCallback(&ChatEventCallback);
	}
	ImGui::ShowHelp("Double clicking on the author of a message in chat will target the author");

	ImGui::Text("Flash Guild Wars taskbar icon when:");
	ImGui::Indent();
	ImGui::ShowHelp("Only triggers when Guild Wars is not the active window");
	if (ImGui::Checkbox("Receiving a private message", &flash_window_on_pm)) {
		GW::Chat::SetWhisperCallback(&WhisperCallback);
	}
	ImGui::Checkbox("Receiving a party invite", &flash_window_on_party_invite);
	ImGui::Checkbox("Zoning in a new map", &flash_window_on_zoning);
	ImGui::Unindent();

	ImGui::Checkbox("Automatically set 'Away' after ", &auto_set_away);
	ImGui::SameLine();
	ImGui::PushItemWidth(50);
	ImGui::InputInt("##awaydelay", &auto_set_away_delay, 0);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("minutes of inactivity");
	ImGui::ShowHelp("Only if you were 'Online'");

	ImGui::Checkbox("Automatically set 'Online' after an input to Guild Wars", &auto_set_online);
	ImGui::ShowHelp("Only if you were 'Away'");
}

void GameSettings::DrawBorderlessSetting() {
	if (ImGui::Checkbox("Borderless Window", &borderless_window)) {
		ApplyBorderless(borderless_window);
	}
}

bool GameSettings::RectEquals(RECT a, RECT b) {
	return  a.left == b.left &&
			a.top == b.top &&
			a.right == b.right &&
			a.bottom == b.bottom;
}

bool GameSettings::RectMultiscreen(RECT desktop, RECT gw) {
	return gw.right > desktop.right || gw.bottom > desktop.bottom;
}


void GameSettings::ApplyBorderless(bool borderless)  {
	Log::Log("ApplyBorderless(%d)\n", borderless);

	borderless_window = borderless;

	HWND gwHandle = GW::MemoryMgr::GetGWWindowHandle();
	DWORD current_style = GetWindowLong(gwHandle, GWL_STYLE);

	RECT windowRect, desktopRect;
	GetWindowRect(gwHandle, &windowRect);
	GetWindowRect(GetDesktopWindow(), &desktopRect);

	//fullscreen or borderless
	if (RectEquals(windowRect, desktopRect)) {
		if ((current_style & WS_MAXIMIZEBOX) != 0) {
			//borderless
			if (borderless) {
				//trying to activate borderless while already in borderless
				Log::Info("Already in Borderless mode");
				return;
			}
		} else {
			//fullscreen
			Log::Info("Please enable Borderless while in Windowed mode");
			borderless = false;
			borderless_window = false;
			return;
		}
	} else if (RectMultiscreen(desktopRect, windowRect)) {
		//borderless multiscreen
		if (borderless) {
			//trying to activate borderless while already in borderless
			Log::Info("Already in Borderless mode (multiscreen)");
		}
	} else if ((current_style & WS_BORDER) == 0 && (current_style & WS_THICKFRAME) == 0) {
		//Borderless with window size smaller than Desktop
		//borderless multiscreen
		if (borderless) {
			//trying to activate borderless while already in borderless
			Log::Info("Already in Borderless mode");
		}
	} else {
		//window
		if (!borderless) {
			//trying to deactivate borderless in window mode
			//Log::Info("Already in Window mode");
			//FIXME for some reason function gets called twice on tb startup with param false.. comment out not to confuse user
		}
	}


	DWORD style = borderless ? WS_POPUP | WS_VISIBLE | WS_MINIMIZEBOX | WS_CLIPSIBLINGS | WS_MAXIMIZEBOX
		: WS_SIZEBOX | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CLIPSIBLINGS;

	//printf("old 0x%X, new 0x%X\n", current_style, style);

	if (current_style != style) {
		for (GW::MemoryPatcher* patch : patches) patch->TooglePatch(borderless);

		SetWindowLong(GW::MemoryMgr::GetGWWindowHandle(), GWL_STYLE, style);

		if (borderless) {
			int width = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);
			MoveWindow(GW::MemoryMgr::GetGWWindowHandle(), 0, 0, width, height, TRUE);
		} else {
			RECT size;
			SystemParametersInfoW(SPI_GETWORKAREA, 0, &size, 0);
			MoveWindow(GW::MemoryMgr::GetGWWindowHandle(), size.top, size.left, size.right, size.bottom, TRUE);
		}
	}
}

void GameSettings::Update() {
	if (auto_set_away 
		&& TIMER_DIFF(activity_timer) > auto_set_away_delay * 60000
		&& GW::FriendListMgr::GetMyStatus() == (DWORD)GW::Constants::OnlineStatus::ONLINE) {
		GW::FriendListMgr::SetFriendListStatus(GW::Constants::OnlineStatus::AWAY);
		activity_timer = TIMER_INIT(); // refresh the timer to avoid spamming in case the set status call fails
	}
}

bool GameSettings::WndProc(UINT Message, WPARAM wParam, LPARAM lParam) {

	activity_timer = TIMER_INIT();
	static clock_t set_online_timer = TIMER_INIT();
	if (auto_set_online
		&& TIMER_DIFF(set_online_timer) > 5000 // to avoid spamming in case of failure
		&& GW::FriendListMgr::GetMyStatus() == (DWORD)GW::Constants::OnlineStatus::AWAY) {
		GW::FriendListMgr::SetFriendListStatus(GW::Constants::OnlineStatus::ONLINE);
		set_online_timer = TIMER_INIT();
	}

	return false;
}
