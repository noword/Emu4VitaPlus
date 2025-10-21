#include "item_achievement.h"

ItemAchievement::ItemAchievement(Achievement *achievement) : _achievement(achievement)
{
}

ItemAchievement::~ItemAchievement()
{
}

void ItemAchievement::Show(bool selected)
{
    ImGui::BeginGroup();
    vita2d_texture *texture = _achievement->GetTexture();
    if (texture)
    {
        ImGui::Image(texture, {vita2d_texture_get_width(texture), vita2d_texture_get_height(texture)});
        ImGui::SameLine();
    }
    ImGui::BeginGroup();
    std::string title = _achievement->title;
    if (_achievement.unlock_time.size() > 0)
    {
        title += " (" + _achievement->unlock_time + ")";
    }

    ImGui::Text(title.c_str());
    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
    ImGui::TextWrapped(_achievement->description);
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::EndGroup();
}