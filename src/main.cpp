#include <Arduino.h>
#include "motor.h"
#include "sensor.h"
#include "pid.h"

#define START_SWITCH_PIN 4

bool isStartSwitchPressed()
{
  return digitalRead(START_SWITCH_PIN) == LOW;
}

void setup()
{
  Serial.begin(115200);
  motorsInit();
  sensorInit();
  pinMode(START_SWITCH_PIN, INPUT_PULLUP);
  calibrateSensors();
  Serial.println("PID test — place on line!");
  Serial.println("Close switch to start");
  delay(2000);
}

void loop()
{
  if (!isStartSwitchPressed())
  {
    stopMotors();
    Serial.println("Switch is open. Waiting to start...");
    delay(200);
    return;
  }

  if (isLineLost())
  {
    stopMotors();
    Serial.println("Line lost! Searching...");

    if (!recoverLine())
    {
      Serial.println("Recovery failed. Waiting for line.");
      delay(200);
      return;
    }

    Serial.println("Line found. Resuming PID.");
  }
  else
  {
    pidFollow();
  }
}