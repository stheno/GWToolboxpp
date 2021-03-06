#pragma once

#include <string>
#include <vector>
#include <functional>
#include <Windows.h>
#include <time.h>

#include <ToolboxModule.h>
#include <ToolboxUIElement.h>

class ChatCommands : public ToolboxModule {
	const float DEFAULT_CAM_SPEED = 25.0f;

	ChatCommands() : move_forward(0), move_side(0), move_up(0),
		cam_speed_(DEFAULT_CAM_SPEED),
		skill_to_use(0), skill_usage_delay(1.0f), skill_timer(clock()) {};
	~ChatCommands() {};
public:
	static ChatCommands& Instance() {
		static ChatCommands instance;
		return instance;
	}

	const char* Name() const override { return "Chat Commands"; }

	void Initialize() override;

	void DrawSettings() override {};

	void DrawHelp();

	bool WndProc(UINT Message, WPARAM wParam, LPARAM lParam);

	// Update. Will always be called every frame.
	void Update() override;

private:
	// 0xA37890 is a mirror of 0xA377C8.
	bool IsTyping() { return (*(DWORD*)0xA377C8) != 0; } 
	
	static std::wstring GetLowerCaseArg(std::vector<std::wstring>, size_t index);
	static std::wstring WStrToLower(std::wstring str);
	static bool ReadTemplateFile(std::wstring path, char *buff, size_t buffSize);

	static void CmdAge2(std::wstring& cmd, std::wstring& args);
	static void CmdDialog(std::wstring& cmd, std::wstring& args);
	static void CmdTB(std::wstring& cmd, std::wstring& args);
	static void CmdTP(std::wstring& cmd, std::wstring& args);
	static void CmdDamage(std::wstring& cmd, std::wstring& args);
	static void CmdChest(std::wstring& cmd, std::wstring& args);
	static void CmdAfk(std::wstring& cmd, std::wstring& args);
	static void CmdTarget(std::wstring& cmd, std::wstring& args);
	static void CmdUseSkill(std::wstring& cmd, std::wstring& args);
	static void CmdShow(std::wstring& cmd, std::wstring& args);
	static void CmdHide(std::wstring& cmd, std::wstring& args);
	static void CmdZoom(std::wstring& cmd, std::wstring& args);
	static void CmdCamera(std::wstring& cmd, std::wstring& args);
	static void CmdSCWiki(std::wstring& cmd, std::wstring& args);
	static void CmdLoad(std::wstring& cmd, std::wstring& args);

	static std::vector<ToolboxUIElement*> MatchingWindows(std::vector<std::wstring>& args);

	int move_forward;
	int move_side;
	int move_up;
	float cam_speed_;

	int skill_to_use;
	float skill_usage_delay;
	clock_t skill_timer;
};
