#include <string>
#include "tab_base.h"
#include "icons.h"

const char *TAB_ICONS[] = {
    ICON_MENU_SPACE,
    ICON_EJECT_SPACE,
    ICON_SAVE_SPACE,
    ICON_ROCKET_SPACE,
    ICON_AWARD_SPACE,
    ICON_FOLDER_SPACE,
    ICON_STAR_SPACE,
    ICON_IMAGE_SPACE,
    ICON_GAMEPAD_SPACE,
    ICON_LINK_SPACE,
    ICON_SLIDER_SPACE,
    ICON_OPTIONS_SPACE,
    ICON_INFO_SPACE,
};

TabBase::TabBase(TEXT_ENUM title_id, bool visable)
    : _title_id(title_id),
      _visable(visable)
{
    LogFunctionName;
    LogDebug(TEXT(_title_id));
}

TabBase::~TabBase()
{
}

void TabBase::Show(bool selected)
{
    std::string title = std::string(TAB_ICONS[_title_id]) + TEXT(_title_id);

    if (ImGui::BeginTabItem(title.c_str(), NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        _Show();
        ImGui::EndTabItem();
    }
}