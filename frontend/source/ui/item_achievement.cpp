#include "item_achievement.h"

ItemAchievement::ItemAchievement(Achievement *achievement) : _achievement(achievement)
{
}

ItemAchievement::~ItemAchievement()
{
}

void ItemAchievement::Show(bool selected)
{
    vita2d_texture *texture = _achievement->GetTexture();
    if (texture)
    {
        ImGui::Image(texture, {vita2d_texture_get_width(texture), vita2d_texture_get_height(texture)});
        ImGui::SameLine();
    }
}