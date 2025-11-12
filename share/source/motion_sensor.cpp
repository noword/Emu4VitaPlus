#include "motion_sensor.h"
#include "log.h"

MotionSensor::MotionSensor() : _enabled(false)
{
}

MotionSensor::~MotionSensor() {}

void MotionSensor::Start()
{
    LogFunctionName;
    sceMotionStartSampling();
    sceMotionMagnetometerOn();
    _enabled = true;
}

void MotionSensor::Stop()
{
    LogFunctionName;
    sceMotionMagnetometerOff();
    sceMotionStopSampling();
    _enabled = false;
}

const SceMotionSensorState &MotionSensor::GetState()
{
    sceMotionGetSensorState(&_state, 1);
    return _state;
}