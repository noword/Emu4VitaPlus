#pragma once
#include <psp2/motion.h>

class MotionSensor
{
public:
    MotionSensor();
    virtual ~MotionSensor();

    void Start();
    void Stop();
    const SceMotionSensorState &GetState();

private:
    SceMotionSensorState _state;
};