#pragma once
#include "thread_base.h"

#define DRAW_IMGUI_TOGETHER

namespace Emu4VitaPlus
{
    class VideoThread : public ThreadBase
    {
    public:
        VideoThread();
        virtual ~VideoThread();

    private:
        static int _DrawThread(SceSize args, void *argp);
    };
}
