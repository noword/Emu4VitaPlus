#include "item_disk.h"
#include "icons.h"
#include "log.h"

#define DISK_ICON_WIDTH 50

// Dialog ItemDisk::_confirm_dialog{LANG_INSERT_DISK, {LANG_OK, LANG_CANCEL}, std::bind(&ItemDisk::_OnRun, this, std::placeholders::_1, std::placeholders::_2)};

ItemDisk::ItemDisk(DiskControl *disk_control, size_t index, InsertCallbackFunc callback)
    : ItemSelectable(LANG_DISK),
      _disk_control(disk_control),
      _index(index),
      _insert_callback(callback)
{
    LogFunctionName;
    _inserted = disk_control->GetImageIndex() == index;
    _info = disk_control->GetImagePath(index);
}

ItemDisk::~ItemDisk()
{
}

void ItemDisk::Show(bool selected)
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    float height = ImGui::GetTextLineHeightWithSpacing();

    if (selected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
    }

    ImGui::Button(_inserted ? ICON_CD : "", {DISK_ICON_WIDTH, height});
    ImGui::SameLine();
    char text[64];
    snprintf(text, 64, "%s %d", _text.Get(), _index);
    ImGui::Button(text, {size.x - DISK_ICON_WIDTH, height});

    if (selected)
    {
        ImGui::PopStyleColor();
        _confirm_dialog.Show();
    }
}

void ItemDisk::OnActive(Input *input)
{
    LogFunctionName;
    if (!_inserted)
    {
        _confirm_dialog.OnActive(input);
    }
}

void ItemDisk::_OnRun(Input *input, int index)
{
    LogFunctionName;
    switch (index)
    {
    case 0: // OK
        if (_disk_control->ReplaceImageIndex(_index))
        {
            _inserted = true;
            _insert_callback(_index);
        }
        break;
    case 1: // Cancel
        break;
    default:
        break;
    }
}