#pragma once
#include "item_selectable.h"
#include "language_string.h"
#include "theme.h"
#include "global.h"
#include "log.h"

class ItemTheme : public ItemSelectable
{
public:
    ItemTheme(const Themes &themes)
        : _themes(themes),
          ItemSelectable(LANG_THEME, "", std::bind(&ItemTheme::_SaveConfig, this))
    {
        size_t dark = 0;
        for (const auto &theme : _themes.Get())
        {
            if (theme.name == gConfig->theme)
            {
                break;
            }
            else if (theme.name == "Dark")
            {
                dark = _index;
            }

            _index++;
        }

        if (_index == _themes.Get().size())
        {
            _index = dark;
        }
    };

    virtual ~ItemTheme() {};

protected:
    virtual size_t _GetTotalCount() override { return _themes.Get().size(); };
    virtual const char *_GetOptionString(size_t index) override { return _themes.Get()[index].name.c_str(); };
    virtual void _OnKeyUp(Input *input) override
    {
        ItemSelectable::_OnKeyUp(input);
        _themes.Apply(_index);
    };

    virtual void _OnKeyDown(Input *input) override
    {
        ItemSelectable::_OnKeyDown(input);
        _themes.Apply(_index);
    };

    virtual void _OnCancel(Input *input) override
    {
        ItemSelectable::_OnCancel(input);
        _themes.Apply(_index);
    };

    void _SaveConfig()
    {
        gConfig->theme = _themes.Get()[_index].name;
        gConfig->Save();
    };

    const Themes &_themes;
};