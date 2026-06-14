#include <Arduino.h>
#include "motor.h"
#include "sensor.h"
#include "pid.h"

void setup()
{
  Serial.begin(115200);
  motorsInit();
  sensorInit();
  calibrateSensors();
  Serial.println("PID test — place on line!");
  delay(2000);
}

void loop()
{
  if (isLineLost())
  {
    stopMotors();
    Serial.println("Line lost!");
  }
  else
  {
    pidFollow();
  }
}