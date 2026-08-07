#pragma once
#include <queue>
#include <string>
#include <imgui_vita2d/imgui_vita.h>
#include "locker.h"
#include "language_string.h"

struct HintItem
{
    LanguageString lang_string;
    int frame_count;
    ImU32 color;
};

class Hint
{
public:
    Hint();
    virtual ~Hint();

    bool NeedShow() { return !_hints.empty(); };
    void Show();
    void SetHint(LanguageString s,
                 int frame_count = 120,
                 bool clear_exists = false,
                 ImU32 color = 0);

private:
    std::queue<HintItem> _hints;
    Locker _locker;
};