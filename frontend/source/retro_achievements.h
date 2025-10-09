#pragma once
#include <rc_client.h>
#include <rc_runtime.h>

class RetroAchievements
{
public:
    RetroAchievements();
    virtual ~RetroAchievements();

private:
    rc_client_t *_client;
};