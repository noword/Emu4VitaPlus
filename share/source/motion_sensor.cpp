#include "motion_sensor.h"
#include "log.h"

MotionSensor::MotionSensor() {}

MotionSensor::~MotionSensor() {}

void MotionSensor::Start()
{
    LogFunctionName;
    sceMotionStartSampling();
    sceMotionMagnetometerOn();
}

void MotionSensor::Stop()
{
    LogFunctionName;
    sceMotionMagnetometerOff();
    sceMotionStopSampling();
}

const SceMotionSensorState &MotionSensor::GetState()
{
    sceMotionGetSensorState(&_state, 1);
    return _state;
}