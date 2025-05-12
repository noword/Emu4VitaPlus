#pragma once

class RetroModule
{
public:
    RetroModule(const char *name);
    virtual ~RetroModule();

private:
    int _id;
};
