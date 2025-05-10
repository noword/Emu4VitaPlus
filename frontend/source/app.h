#pragma once
#include "video.h"
#include "log.h"

class App
{
public:
    App(int argc, char *const argv[]);
    virtual ~App();

    void Run();

private:
    bool _IsSaveMode();
};