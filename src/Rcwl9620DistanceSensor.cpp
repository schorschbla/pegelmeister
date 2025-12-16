#include "Rcwl9620DistanceSensor.h"

const uint8_t Rcwl9620CommandStartMeasure = 0xA0;
const unsigned int Rcwl9620OutOfRangeValue = 7998840;

Rcwl9620DistanceSensor::Rcwl9620DistanceSensor(HardwareSerial &serial, unsigned int readTimeoutMs)
    : serial(serial), readTimeoutMs(readTimeoutMs), lastStartMeasureTime(0)
{
}

static void clearRxBuffer(HardwareSerial &serial)
{
  while (serial.available())
  {
    serial.read();
  }
}

void Rcwl9620DistanceSensor::startMeasure()
{
  serial.flush();
  serial.write(Rcwl9620CommandStartMeasure);
  clearRxBuffer(serial);
  lastStartMeasureTime = millis();
}

Rcwl9620DistanceSensor::MeasureState Rcwl9620DistanceSensor::measureState() const
{
  Rcwl9620DistanceSensor::MeasureState state;
  if (lastStartMeasureTime == 0)
  {
    state = Rcwl9620DistanceSensor::MeasureState::Idle;
  }
  else if (serial.available() == 3)
  {
    state = Rcwl9620DistanceSensor::MeasureState::Completed;
  }
  else if (millis() - lastStartMeasureTime > 150)
  {
    state = Rcwl9620DistanceSensor::MeasureState::Timeout;
  }
  else
  {
    state = Rcwl9620DistanceSensor::MeasureState::Started;
  }
  return state;
}

Rcwl9620DistanceSensor::MeasureResult Rcwl9620DistanceSensor::readDistance(unsigned int &distance)
{
  if (serial.available() != 3)
  {
    return Rcwl9620DistanceSensor::MeasureResult::NotCompleted;
  }

  lastStartMeasureTime = 0;

  byte response[3];
  serial.readBytes(response, 3);

  distance = (response[0] << 16) | (response[1] << 8) | response[2];
  return distance < Rcwl9620OutOfRangeValue ? 
    Rcwl9620DistanceSensor::MeasureResult::Success : Rcwl9620DistanceSensor::MeasureResult::OutOfRange;
}
