#include "tab_disk.h"
#include "item_disk.h"

TabDisk::TabDisk(DiskControl *disk_control)
    : TabSeletable::TabSeletable(LANG_DISK, 1),
      _disk_control(disk_control)
{
    LogFunctionName;
    LogDebug("images: %d index: %d", disk_control->GetNumImages(), disk_control->GetImageIndex());

    size_t total = disk_control->GetNumImages();
    _items.reserve(total);
    for (size_t i = 0; i < total; i++)
    {
        _items.emplace_back(new ItemDisk(disk_control, i, std::bind(&TabDisk::_InsertCallback, this, std::placeholders::_1)));
    }

    _current_index = disk_control->GetImageIndex();
}

TabDisk::~TabDisk()
{
}

void TabDisk::_InsertCallback(size_t index)
{
    LogFunctionName;
    ((ItemDisk *)_items[_current_index])->Eject();
    _current_index = index;
}