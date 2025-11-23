#pragma once
#include "input.h"
#include "thread_base.h"

using namespace Emu4VitaPlus;

class InputThread
    : virtual public Input,
      virtual public ThreadBase
{
public:
    InputThread() : ThreadBase(_InputThread) {};
    virtual ~InputThread() {};

    bool Start()
    {
        LogFunctionName;

        uint32_t p = (uint32_t)this;
        return ThreadBase::Start(&p, 4);
    }

private:
    static int _InputThread(SceSize args, void *argp)
    {
        CLASS_POINTER(InputThread, input, argp);

        while (input->IsRunning())
        {
            input->Wait();
            input->Poll();
        }

        return 0;
    }
};