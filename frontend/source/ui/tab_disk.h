#pragma once
#include "tab_selectable.h"
#include "disk_contorl.h"

class TabDisk : public virtual TabSeletable
{
public:
    TabDisk(DiskControl *disk_control);
    virtual ~TabDisk();

    // virtual void SetInputHooks(Input *input) override;
    // virtual void UnsetInputHooks(Input *input) override;
    virtual void Show(bool selected) override;

private:
    // virtual void _OnActive(Input *input) override;
    virtual size_t _GetItemCount() override { return _disk_control->GetNumImages(); };
    DiskControl *_disk_control;
    size_t _index;
    bool _inserted;
};