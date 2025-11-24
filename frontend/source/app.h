#pragma once
#include "log.h"

class App
{
public:
    App();
    virtual ~App();

    void Run();

private:
    void _CheckVersion();
    bool _IsSaveMode();
};