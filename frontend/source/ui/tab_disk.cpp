#include "tab_disk.h"

TabDisk::TabDisk(DiskControl *disk_control)
    : TabSeletable::TabSeletable(LANG_DISK),
      _disk_control(disk_control)
{
    LogFunctionName;
    LogDebug("images: %d index: %d", disk_control->GetNumImages(), disk_control->GetImageIndex());
}

TabDisk::~TabDisk()
{
}

void TabDisk::Show(bool selected)
{
    std::string title = std::string(TAB_ICONS[_title_id]) + TEXT(_title_id);
    if (ImGui::BeginTabItem(title.c_str(), NULL, selected ? ImGuiTabItemFlags_SetSelected : 0))
    {
        ImGui::EndTabItem();
    }
}