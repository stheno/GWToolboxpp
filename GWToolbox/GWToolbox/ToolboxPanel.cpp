#include "ToolboxPanel.h"

#include <Windows\MainWindow.h>

void ToolboxPanel::Initialize() {
	ToolboxWindow::Initialize();
	MainWindow::Instance().RegisterPanel(this);
}

bool ToolboxPanel::DrawTabButton(IDirect3DDevice9* device) {
	ImGui::PushStyleColor(ImGuiCol_Button, visible ?
		ImGui::GetStyle().Colors[ImGuiCol_Button] : ImVec4(0, 0, 0, 0));
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 textsize = ImGui::CalcTextSize(Name());
	float width = ImGui::GetWindowContentRegionWidth();
	float img_size = texture ? ImGui::GetTextLineHeightWithSpacing() : 0.0f;
	float text_x = pos.x + img_size + (width - img_size - textsize.x) / 2;
	bool clicked = ImGui::Button("", ImVec2(width, ImGui::GetTextLineHeightWithSpacing()));
	if (texture != nullptr) {
		ImGui::GetWindowDrawList()->AddImage((ImTextureID)texture, pos,
			ImVec2(pos.x + img_size, pos.y + img_size));
	}
	ImGui::GetWindowDrawList()->AddText(ImVec2(text_x, pos.y + ImGui::GetStyle().ItemSpacing.y / 2),
		ImColor(ImGui::GetStyle().Colors[ImGuiCol_Text]), Name());
	if (clicked) visible = !visible;
	ImGui::PopStyleColor();
	return clicked;
}
