#pragma once
#include <stdint.h>

template <typename T>
class Delay
{
public:
    Delay() {};
    Delay(T interval_ms) { SetInterval(interval_ms); };
    virtual ~Delay() {};

    void SetInterval(T interval_ms)
    {
        _interval_ms = interval_ms;
        _next_ms = sceKernelGetProcessTimeWide() + _interval_ms;
    };

    T GetInterval() { return _interval_ms; };

    bool Wait()
    {
        T current = sceKernelGetProcessTimeWide();
        bool result = (current <= _next_ms);

        if (current < _next_ms)
        {
            sceKernelDelayThread(_next_ms - current);
            _next_ms += _interval_ms;
        }
        else
        {
            _next_ms = current + _interval_ms;
        }

        return result;
    }

    bool TimeUp()
    {
        T current = sceKernelGetProcessTimeWide();
        bool result = (current >= _next_ms);
        if (result)
        {
            _next_ms = current + _interval_ms;
        }
        return result;
    }

private:
    T _interval_ms;
    T _next_ms;
};