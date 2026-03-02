#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <RunningMedian.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "tahoma22pt7b.h"
#include "tahoma24pt7b.h"
#include "tahoma21pt7b.h"
#include "tahoma20pt7b.h"
#include "tahoma19pt7b.h"
#include "tahoma18pt7b.h"
#include "tahoma12pt7b.h"

#include "Rcwl9620DistanceSensor.h"

Rcwl9620DistanceSensor sensor(Serial1);
RunningMedian average(511);
Adafruit_SH1107 display = Adafruit_SH1107(128, 128, &Wire, -1, 1000000, 100000);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 600000);

unsigned int wifiLastConnectionAttempt;

unsigned int diameter = 1500;
unsigned int highMark = 400;
unsigned int lowMark = 2500;

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
  display.setTextColor(SH110X_WHITE);

  display.setFont(&FreeSans9pt7b);
  display.setCursor(8, 40);
  display.print("Fuellstand:");

  average.clear();

  WiFi.begin("Bluesheep", "6aKH5wJx&3");
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  wifiLastConnectionAttempt = millis();

  timeClient.begin();

  Serial.println("Startgin...");

}

unsigned int timeLastUpdated = 0;

void loop()
{
  Rcwl9620DistanceSensor::MeasureState sensorState = sensor.measureState();
  if (sensorState == Rcwl9620DistanceSensor::MeasureState::Completed)
  {
    unsigned int distance;
    if (sensor.readDistance(distance) == Rcwl9620DistanceSensor::MeasureResult::Success)
    {
      average.add(distance / 10000.0);

      int distanceMm = average.getAverage() * 10;

      float ratio = (lowMark - distanceMm) / (float)(lowMark - highMark);

      char tmp[32];
      snprintf(tmp, 16, "%.1f %%", ratio * 100);

      display.fillRect(0, 41, 128, 80, SH110X_BLACK);
      display.setFont(&FreeSans18pt7b);
 display.setCursor(4, 72);     
      display.print(tmp);

      float volumeLiters = PI * pow(diameter * 0.5, 2) * (lowMark - distanceMm) / 1000000;

      snprintf(tmp, 16, "%d", (int) volumeLiters);
      display.setCursor(4, 105);     
      display.print(tmp);
      display.display();
    }
  }
  if (sensorState != Rcwl9620DistanceSensor::MeasureState::Started)
  {
    sensor.startMeasure();
  }

  wl_status_t wifiStatus = WiFi.status();
  if (wifiStatus != WL_CONNECTED && millis() - wifiLastConnectionAttempt > 10000) 
  {
      Serial.printf("Reconnect %d\n", wifiStatus);
      WiFi.reconnect();
      wifiLastConnectionAttempt = millis();
  }

  if (timeLastUpdated == 0 || millis() - timeLastUpdated > 10000)
  {
    if (wifiStatus == WL_CONNECTED)
    {
      timeClient.update();
    }
 
    display.fillRect(0, 0, 128, 17, SH110X_BLACK);
    display.setFont(&FreeSans9pt7b);
    display.setCursor(8, 16);

    if (timeClient.isTimeSet())
    {
      time_t now = timeClient.getEpochTime();
      struct tm* timeinfo = localtime(&now);
      char buf[19];
      strftime (buf, sizeof(buf), "%d.%m. %R", timeinfo);
      Serial.println(buf);
      display.print(buf);
    }
    else
    {
      display.print("Kein Wlan");
    }

    display.display();

    timeLastUpdated = millis();
  }

  delay(4);
}
