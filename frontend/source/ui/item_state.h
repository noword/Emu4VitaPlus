#pragma once
#include "item_selectable.h"
#include "state_manager.h"
#include "dialog.h"

enum
{
    POPUP_SAVE = 0,
    POPUP_LOAD,
    POPUP_DELETE,
    POPUP_CANCEL,
    POPUP_COUNT,
};

class ItemState : public ItemSelectable
{
public:
    ItemState(State *state);
    virtual ~ItemState();
    virtual void SetInputHooks(Input *input) override;
    virtual void UnsetInputHooks(Input *input) override;
    virtual void Show(bool selected) override;
    virtual void OnActive(Input *input) override;

private:
    void _ShowPopup();
    // void _ShowDialog();

    virtual size_t _GetTotalCount() override { return _menu_count; };
    void _OnKeyLeft(Input *input);
    void _OnKeyRight(Input *input);
    virtual void _OnClick(Input *input) override;
    virtual void _OnCancel(Input *input) override;
    virtual void _OnRun(Input *input, int index);

    State *_state;
    Dialog *_confirm_dialog;
    const TEXT_ENUM *_menu_texts;
    size_t _menu_count;
};