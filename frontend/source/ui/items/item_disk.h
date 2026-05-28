#pragma once
#include "item_selectable.h"
#include "disk_control.h"
#include "dialog.h"

typedef std::function<void(int)> InsertCallbackFunc;

class ItemDisk : public ItemSelectable
{
public:
    ItemDisk(DiskControl *disk_control, size_t index, InsertCallbackFunc callback);
    virtual ~ItemDisk();

    virtual void Show(bool selected) override;
    virtual void OnActive(Input *input) override;
    void Eject() { _inserted = false; };

private:
    virtual size_t _GetTotalCount() override { return 0; };
    virtual void _OnRun(Input *input, int index);

    DiskControl *_disk_control;
    bool _inserted;
    size_t _index;
    InsertCallbackFunc _insert_callback;

    Dialog _confirm_dialog{LANG_INSERT_DISK, {LANG_OK, LANG_CANCEL}, std::bind(&ItemDisk::_OnRun, this, std::placeholders::_1, std::placeholders::_2)};
};