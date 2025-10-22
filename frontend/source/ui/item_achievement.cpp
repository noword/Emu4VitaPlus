#include "item_achievement.h"
#include "log.h"

ItemAchievement::ItemAchievement(Achievement *achievement)
    : ItemBase(achievement->title, achievement->description),
      _achievement(achievement)
{
}

ItemAchievement::~ItemAchievement()
{
}

void ItemAchievement::Show(bool selected)
{
    // LogDebug("000");
    ImGui::BeginGroup();
    vita2d_texture *texture = _achievement->GetTexture();
    if (texture)
    {
        ImGui::Image(texture, {(float)vita2d_texture_get_width(texture), (float)vita2d_texture_get_height(texture)});
        ImGui::SameLine();
    }
    ImGui::BeginGroup();
    ImGui::Selectable(_achievement->title.c_str(), selected);
    if (_achievement->unlock_time.size() > 0)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
        ImGui::Text(_achievement->unlock_time.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();
    ImGui::EndGroup();
    // LogDebug("end");
}