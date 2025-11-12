#pragma once
#include <psp2/motion.h>

class MotionSensor
{
public:
    MotionSensor();
    virtual ~MotionSensor();

    void Start();
    void Stop();
    bool Enabled() { return _enabled; };
    const SceMotionSensorState &GetState();

private:
    SceMotionSensorState _state;
    bool _enabled;
};