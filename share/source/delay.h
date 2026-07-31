#pragma once
#include <stdint.h>
#include <psp2/kernel/processmgr.h>
#include "utils.h"

template <typename T>
class Delay
{
public:
    Delay() {};
    Delay(T interval_ms, T start_ms = 0) { SetInterval(interval_ms, start_ms); };

    virtual ~Delay() {};

    void SetInterval(T interval_ms, T start_ms = 0)
    {
        _interval_ms = interval_ms;
        _outtime_ms = interval_ms * 8;
        _next_ms = sceKernelGetProcessTimeWide() + _interval_ms + start_ms;
    };

    T GetInterval() { return _interval_ms; };

    void Wait()
    {
        T current = sceKernelGetProcessTimeWide();

        if (likely(current < _next_ms))
        {
            sceKernelDelayThread(_next_ms - current);
            _next_ms += _interval_ms;
        }
        else if (current > _next_ms + _outtime_ms)
        {
            _next_ms = current + _interval_ms;
        }
        else
        {
            _next_ms += _interval_ms;
        }
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

protected:
    T _interval_ms;
    T _next_ms;
    T _outtime_ms;
};

template <typename T>
class LosseDelay : public Delay<T>
{
public:
    void SetInterval(T interval_ms, T start_ms = 0, float fluctuation_threshold = 0.05)
    {
        Delay<T>::SetInterval(interval_ms, start_ms);
        _fluctuation = interval_ms * fluctuation_threshold;
    }

    bool Wait()
    {
        T current = sceKernelGetProcessTimeWide();

        if (likely(current < _next_ms))
        {
            sceKernelDelayThread(_next_ms - current);
            _next_ms += _interval_ms;
            return true;
        }
        else if (unlikely(current > _next_ms + _outtime_ms))
        {
            _next_ms = current + _interval_ms;
            return false;
        }
        else
        {
            bool result = current < (_next_ms + _fluctuation);
            _next_ms += _interval_ms;
            return result;
        }
    }

private:
    using Delay<T>::_interval_ms;
    using Delay<T>::_next_ms;
    using Delay<T>::_outtime_ms;

    T _fluctuation;
};