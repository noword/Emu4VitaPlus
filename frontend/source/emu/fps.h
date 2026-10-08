#pragma once
#include <stdint.h>

class Fps
{
public:
    Fps();
    virtual ~Fps();

    void Update();
    float Get() { return _fps; };
    void Show();

private:
    float _fps;
    uint64_t _last_time;
    int _frame_count;
};