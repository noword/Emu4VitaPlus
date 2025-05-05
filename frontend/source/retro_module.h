#pragma once
#include "core.h"

class RetroModule
{
public:
    RetroModule(const char *name);
    virtual ~RetroModule();

private:
    int _id;
};

extern RetroCore *gRetro;