#include "ToolboxWindow.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "ImGuiAddons.h"

#include <Modules\ToolboxSettings.h>

void ToolboxWindow::DrawSettings() {
	if (ImGui::CollapsingHeader(Name(), ImGuiTreeNodeFlags_AllowOverlapMode)) {
		ImGui::PushID(Name());
		ShowVisibleRadio();
		ImVec2 pos(0, 0);
		if (ImGuiWindow* window = ImGui::FindWindowByName(Name())) pos = window->Pos;
		if (ImGui::DragFloat2("Position", (float*)&pos, 1.0f, 0.0f, 0.0f, "%.0f")) {
			ImGui::SetWindowPos(Name(), pos);
		}
		ImGui::ShowHelp("You need to show the window for this control to work");
		ImGui::Checkbox("Lock Position", &lock_move);
		ImGui::SameLine();
		ImGui::Checkbox("Lock Size", &lock_size);
		ImGui::SameLine();
		ImGui::Checkbox("Show close button", &show_closebutton);
		DrawSettingInternal();
		ImGui::PopID();
	} else {
		ShowVisibleRadio();
	}
}

ImGuiWindowFlags ToolboxWindow::GetWinFlags(ImGuiWindowFlags flags) const {
	if (!ToolboxSettings::clamp_window_positions) flags |= ImGuiWindowFlags_NoClampPosition;
	if (ToolboxSettings::move_all) {
		flags |= ImGuiWindowFlags_ShowBorders;
	} else {
		if (lock_move) flags |= ImGuiWindowFlags_NoMove;
		if (lock_size) flags |= ImGuiWindowFlags_NoResize;
	}
	return flags;
}
