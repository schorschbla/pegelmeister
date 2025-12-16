#pragma once

#include <Arduino.h>

class Rcwl9620DistanceSensor
{
public:
    Rcwl9620DistanceSensor(HardwareSerial &serial, unsigned int readTimeoutMs = 4);

    enum MeasureState
    {
       Idle,
       Started,
       Completed,
       Timeout
    };

    enum MeasureResult
    {
       Success,
       OutOfRange,
       NotCompleted
    };

    void startMeasure();
    MeasureState measureState() const;
    MeasureResult readDistance(unsigned int &distance);

private:
    HardwareSerial &serial;
    unsigned int lastStartMeasureTime;
    unsigned int readTimeoutMs;
};