#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <RunningMedian.h>

#include "tahoma22pt7b.h"
#include "tahoma24pt7b.h"
#include "tahoma21pt7b.h"
#include "tahoma20pt7b.h"
#include "tahoma19pt7b.h"
#include "tahoma18pt7b.h"
#include "tahoma12pt7b.h"

#include "Rcwl9620DistanceSensor.h"

Rcwl9620DistanceSensor sensor(Serial1);
RunningMedian average(127);
Adafruit_SH1107 display = Adafruit_SH1107(128, 128, &Wire, -1, 1000000, 100000);

void setup()
{
  Serial.begin(9600);

  Serial1.setPins(20, 21);
  Serial1.begin(9600);

  Wire.setPins(6, 7);
  Wire.begin();

  delay(1000);

  display.begin(0x3c, false);
  display.setRotation(3);
  display.clearDisplay();
  display.setFont(&Tahoma18pt7b);
  display.setTextColor(SH110X_WHITE);

  average.clear();
}

void loop()
{
  Rcwl9620DistanceSensor::MeasureState sensorState = sensor.measureState();
  if (sensorState == Rcwl9620DistanceSensor::MeasureState::Completed)
  {
    unsigned int distance;
    if (sensor.readDistance(distance) == Rcwl9620DistanceSensor::MeasureResult::Success)
    {
      average.add(distance / 10000.0);

      char tmp[32];
      snprintf(tmp, 16, "%.1f%%", average.getAverage());

      display.clearDisplay();
      display.setCursor(0, 32);
      display.print(tmp);
      display.display();
    }
  }
  if (sensorState != Rcwl9620DistanceSensor::MeasureState::Started)
  {
    sensor.startMeasure();
  }
  delay(4);
}
