#pragma once
#include "global.h"
#include "tab_selectable.h"
#include "gamepad.h"

class TabControl : public TabSeletable
{
public:
    TabControl(std::vector<ItemBase *> items) : TabSeletable{LANG_CONTROL, items} {};
    virtual ~TabControl() {};

    virtual void SetInputHooks(Input *input) override
    {
        TabSeletable::SetInputHooks(input);
        input->SetKeyDownCallback(SCE_CTRL_START, std::bind(&TabControl::_OnStart, this, input));
    };

    virtual void UnsetInputHooks(Input *input) override
    {
        TabSeletable::UnsetInputHooks(input);
        input->UnsetKeyDownCallback(SCE_CTRL_START);
    }

private:
    void _OnStart(Input *input)
    {
        input->PushCallbacks();
        gEmulator->SetupKeys();
        gEmulator->StartInput();
        _gamepad.SetInput(input);
        _gamepad.SetActive(true);
    }

    virtual void _Show() override
    {
        TabSeletable::_Show();
        _gamepad.Show();
    };

    Emu4VitaPlus::Gamepad _gamepad;
};