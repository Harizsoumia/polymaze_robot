#pragma once
#include <Arduino.h>
#include "motor.h"
#include "sensor.h"

float Kp = 0.08;
float Ki = 0.0;
float Kd = 0.3;

float lastError = 0;
float integral = 0;
int CENTER = 3500;

void resetPID()
{
    lastError = 0;
    integral = 0;
}

void pidFollow()
{

    int16_t pos = readLine();

    float error = pos - CENTER;

    integral = integral + error;
    float derivative = error - lastError;

    integral = constrain(integral, -10000, 10000);

    float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);

    int leftSpeed = BASE_SPEED + correction;
    int rightSpeed = BASE_SPEED - correction;

    leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
    rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

    setMotorA(leftSpeed);
    setMotorB(rightSpeed);

    lastError = error;

    Serial.print("Pos: ");
    Serial.print(pos);
    Serial.print(" Err: ");
    Serial.print(error);
    Serial.print(" Cor: ");
    Serial.println(correction);
}

bool recoverLine()
{
    float recoveryDirection = lastError;
    resetPID();
    const int recoverySpeed = 90;
    const int sweepTime = 80;

    if (recoveryDirection >= 0)
    {
        turnRight(recoverySpeed);
    }
    else
    {
        turnLeft(recoverySpeed);
    }

    delay(sweepTime);
    stopMotors();
    if (isLineDetected())
        return true;

    if (recoveryDirection >= 0)
    {
        turnLeft(recoverySpeed);
    }
    else
    {
        turnRight(recoverySpeed);
    }

    delay(sweepTime * 2);
    stopMotors();
    if (isLineDetected())
        return true;

    driveForward(recoverySpeed / 2);
    delay(sweepTime);
    stopMotors();
    return isLineDetected();
}