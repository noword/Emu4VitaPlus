#include "item_achievement.h"
#include "log.h"

#define DEFAULT_TEXTURE_HEIGHT 64

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
    ImGui::BeginGroup();
    vita2d_texture *texture = _achievement->GetTexture();
    if (texture)
    {
        ImGui::Image(texture, {(float)vita2d_texture_get_width(texture), (float)vita2d_texture_get_height(texture)});
        ImGui::SameLine();
    }
    ImGui::BeginGroup();
    if (selected)
    {
        ImVec2 size = ImGui::GetContentRegionAvail();
        if (texture)
            size.y = vita2d_texture_get_height(texture);
        else
            size.y = DEFAULT_TEXTURE_HEIGHT;
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        ImVec2 p_min = ImGui::GetCursorScreenPos();
        ImGui::Dummy(size);
        ImVec2 p_max = ImGui::GetItemRectMax();
        draw_list->AddRectFilled(p_min, p_max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered)));
        ImGui::SetCursorScreenPos(p_min);
    }

    ImGui::Text(_achievement->title.c_str());
    if (_achievement->unlock_time.size() > 0)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, {0.8f, 0.8f, 0.8f, 1.0f});
        ImGui::Text(_achievement->unlock_time.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::EndGroup();
    ImGui::EndGroup();
}