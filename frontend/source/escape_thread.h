#include "delay.h"
#include "thread_base.h"

class EscapeThread : public ThreadBase
{
public:
    EscapeThread() : ThreadBase(_EscapeThread, "emergency") {};
    virtual ~EscapeThread() {};

private:
    static int _EscapeThread(SceSize args, void *argp);
};