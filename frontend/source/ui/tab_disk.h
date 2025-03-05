#pragma once
#include "tab_selectable.h"
#include "disk_control.h"

class TabDisk : public virtual TabSeletable
{
public:
    TabDisk(DiskControl *disk_control);
    virtual ~TabDisk();

private:
    void _InsertCallback(size_t index);

    DiskControl *_disk_control;
    size_t _current_index;
};