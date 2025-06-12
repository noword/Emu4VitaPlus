#pragma once
#include <vector>
#include "tab_base.h"
#include "item_base.h"
#include "my_imgui.h"

class TabSeletable : public TabBase
{
public:
    TabSeletable(TEXT_ENUM title_id, std::vector<ItemBase *> items, int columns = 2, float column_ratio = -1.);
    TabSeletable(TEXT_ENUM title_id, int columns = 2);
    virtual ~TabSeletable();

    virtual void SetInputHooks(Input *input) override;
    virtual void UnsetInputHooks(Input *input) override;
    void SetStatusText(std::string text);
    void SetItemVisable(size_t index, bool visable);
    void SetColumns(int columns);
    void SetIndex(size_t index);
    bool Visable() override { return _visable && _GetItemCount() > 0; };

protected:
    virtual void _Show() override;

    virtual void _OnKeyUp(Input *input);
    virtual void _OnKeyDown(Input *input);
    virtual void _OnKeyLeft(Input *input);
    virtual void _OnKeyRight(Input *input);

    virtual size_t _GetItemCount();
    virtual void _ShowItem(size_t index, bool selected);
    virtual void _OnActive(Input *input);
    virtual void _OnOption(Input *input);
    virtual bool _ItemVisable(size_t index);
    virtual void _Update();

    std::vector<ItemBase *> _items;
    size_t _index;
    std::string _status_text;
    int _columns;
    float _column_ratio;
    TextMovingStatus _moving_status;

    My_Imgui_SpinText _spin_text;
    bool _in_refreshing;
};