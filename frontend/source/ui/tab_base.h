#pragma once
#include <imgui_vita2d/imgui_vita.h>
#include <stdint.h>
#include "language_define.h"
#include "language_string.h"
#include "input.h"
#include "log.h"

using namespace Emu4VitaPlus;

enum
{
    TAB_INDEX_SYSTEM = 0,
    TAB_INDEX_DISK,
    TAB_INDEX_STATE,
    TAB_INDEX_CHEAT,
    TAB_INDEX_BROWSER,
    TAB_INDEX_FAVORITE,
    TAB_INDEX_GRAPHICS,
    TAB_INDEX_CONTROL,
    TAB_INDEX_HOTKEY,
    TAB_INDEX_CORE,
    TAB_INDEX_OPTIONS,
    TAB_INDEX_ABOUT,
    TAB_INDEX_COUNT
};

extern const char *TAB_ICONS[];

class TabBase
{
public:
    TabBase(TEXT_ENUM title_id, bool visable = true);

    virtual ~TabBase();

    virtual void Show(bool selected);
    virtual void SetInputHooks(Input *input) {};
    virtual void UnsetInputHooks(Input *input) {};
    virtual void ChangeLanguage(uint32_t language) {};

    void SetVisable(bool visable) { _visable = visable; };
    virtual bool Visable() { return _visable; };

protected:
    TEXT_ENUM _title_id;
    bool _visable;
};