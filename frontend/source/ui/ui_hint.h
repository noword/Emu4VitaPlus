#pragma once
#include <queue>
#include <string>
#include <psp2/kernel/threadmgr.h>
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

    void Show();
    void SetHint(LanguageString s, int frame_count = 120, bool clear_queue = false);

private:
    void _Lock();
    void _Unlock();
    std::queue<HintItem> _hints;
    SceKernelLwMutexWork _mutex;
};