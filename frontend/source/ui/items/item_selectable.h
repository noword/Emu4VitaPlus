#pragma once
#include "item_base.h"

class ItemSelectable : public ItemBase
{
public:
    ItemSelectable(const LanguageString text,
                   LanguageString info = "",
                   CallbackFunc active_callback = nullptr,
                   CallbackFunc option_callback = nullptr);
    virtual ~ItemSelectable();

    virtual void SetInputHooks(Input *input);
    virtual void UnsetInputHooks(Input *input);
    virtual void Show(bool selected) override;
    virtual void OnActive(Input *input) override;

protected:
    virtual void _OnKeyUp(Input *input);
    virtual void _OnKeyDown(Input *input);
    virtual void _OnClick(Input *input);
    virtual void _OnCancel(Input *input);

    virtual size_t _GetTotalCount() = 0;
    virtual const char *_GetOptionString(size_t index) { return ""; };
    virtual const char *_GetPreviewText() { return _GetOptionString(_GetIndex()); };
    virtual size_t _GetIndex() { return _index; };
    virtual void _SetIndex(size_t index) { _index = index; };
    virtual bool _IsHighlight(size_t index) { return index == _GetIndex(); };
    bool _IsOnOff();
    void _ShowCombo(const char *text);
    void _ShowOnOff(bool on);

    bool _actived;
    size_t _index;
    size_t _old_index;
};