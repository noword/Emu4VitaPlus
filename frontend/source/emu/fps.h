#pragma once
#include <stdint.h>

class Fps
{
public:
    Fps();
    virtual ~Fps();

    void Update();
    int Get() { return _fps; };

private:
    int _fps;
    uint64_t _last_time;
    int _frame_count;
};