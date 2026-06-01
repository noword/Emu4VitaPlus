#pragma once
#include "item_selectable.h"
#include "core_options.h"
#include "log.h"
#include "file.h"
#include "global.h"

class ItemCore : public ItemSelectable
{
public:
    ItemCore(CoreOption *option)
        : _option(option),
          ItemSelectable(option->desc,
                         option->info,
                         []()
                         {
                             gConfig->Save();
                             if (*gEmulator->GetCurrentName()) // game loaded
                             {
                                 gEmulator->CoreOptionUpdate();
                                 gHint->SetHint(TEXT(LANG_CORE_NOTICE));
                             }
                         })
    {
        _values = _option->GetValues();
        _index = _option->GetValueIndex();
    };

    virtual ~ItemCore() {};

private:
    virtual size_t _GetTotalCount() override
    {
        return _values.size();
    };

    virtual const char *_GetOptionString(size_t index) override
    {
        return index < _values.size() ? _values[index].Get() : "Invalid";
    };

    virtual void _SetIndex(size_t index) override
    {
        _index = index;
        _option->SetValueIndex(index);
    };

    virtual size_t _GetIndex() override
    {
        return _option->GetValueIndex();
    }

    CoreOption *_option;
    std::vector<LanguageString> _values;
};