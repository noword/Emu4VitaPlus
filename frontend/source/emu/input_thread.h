#pragma once
#include "input.h"
#include "thread_base.h"

using namespace Emu4VitaPlus;

class InputThread
    : virtual public Input,
      virtual public ThreadBase
{
public:
    InputThread();
    virtual ~InputThread();

    bool Start();

private:
    static int _InputThread(SceSize args, void *argp);
};