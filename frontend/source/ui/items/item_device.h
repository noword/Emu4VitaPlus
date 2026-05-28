#pragma once
#include "emulator.h"
#include "ui.h"
#include "item_selectable.h"
#include "device_options.h"
#include "log.h"

class ItemDevice : public ItemSelectable
{
public:
    ItemDevice(const LanguageString text, ControllerTypes *types)
        : ItemSelectable(text, "", std::bind(&Emulator::SetupKeysWithSaveConfig, gEmulator)),
          _types(types)
    {
        _values = _types->GetValues();
        _index = _types->GetValueIndex();
    };
    virtual ~ItemDevice() {};

private:
    virtual size_t _GetTotalCount() override
    {
        return _values.size();
    };

    virtual const char *_GetOptionString(size_t index) override
    {
        return _index < _values.size() ? _values[index].Get() : "Invalid";
    };

    virtual void _SetIndex(size_t index) override
    {
        _index = index;
        _types->SetValueIndex(index);
    };

    ControllerTypes *_types;
    std::vector<LanguageString> _values;
};