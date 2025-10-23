#pragma once
#include <queue>
#include <string>
#include "locker.h"
#include "language_string.h"

struct HintItem
{
    LanguageString lang_string;
    int frame_count;
};

class Hint
{
public:
    Hint();
    virtual ~Hint();

    bool NeedShow() { return !_hints.empty(); };
    void Show();
    void SetHint(LanguageString s, int frame_count = 120, bool clear_exists = false);

private:
    std::queue<HintItem> _hints;
    Locker _locker;
};