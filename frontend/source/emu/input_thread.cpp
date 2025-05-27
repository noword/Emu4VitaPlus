#include "input_thread.h"

InputThread::InputThread() : ThreadBase(_InputThread)
{
}

InputThread::~InputThread()
{
}

int InputThread::_InputThread(SceSize args, void *argp)
{
    LogFunctionName;

    CLASS_POINTER(InputThread, input, argp);

    while (input->IsRunning())
    {
        input->Wait();
        input->Poll();
    }

    LogDebug("_InputThread end");

    return 0;
}

bool InputThread::Start()
{
    LogFunctionName;

    uint32_t p = (uint32_t)this;
    return ThreadBase::Start(&p, 4);
}
