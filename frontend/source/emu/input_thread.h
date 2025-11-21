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