#pragma once
#include "thread_base.h"

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
