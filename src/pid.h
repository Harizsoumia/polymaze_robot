#pragma once
#include <Arduino.h>
#include "motor.h"
#include "sensor.h"

// ─── PID CONSTANTS ───────────────────────────────────────────
// Start with these values, tune after first test
float Kp = 0.08;
float Ki = 0.0;
float Kd = 0.3;

// ─── PID GLOBALS ─────────────────────────────────────────────
float lastError = 0;
float integral = 0;
int CENTER = 3500; // middle of 0-7000 range

// ─────────────────────────────────────────────────────────────
void pidFollow()
{
    // 1. Read current position
    int16_t pos = readLine();

    // 2. Calculate error (how far from center)
    float error = pos - CENTER;

    // 3. Calculate PID terms
    integral = integral + error;
    float derivative = error - lastError;

    // 4. Clamp integral to prevent windup
    integral = constrain(integral, -10000, 10000);

    // 5. Calculate correction
    float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);

    // 6. Apply correction to motors
    int leftSpeed = BASE_SPEED - correction;
    int rightSpeed = BASE_SPEED + correction;

    // 7. Clamp speeds to safe range
    leftSpeed = constrain(leftSpeed, -MAX_SPEED, MAX_SPEED);
    rightSpeed = constrain(rightSpeed, -MAX_SPEED, MAX_SPEED);

    // 8. Drive motors
    setMotorA(leftSpeed);
    setMotorB(rightSpeed);

    // 9. Save error for next iteration
    lastError = error;

    // Debug — comment out after tuning
    Serial.print("Pos: ");
    Serial.print(pos);
    Serial.print(" Err: ");
    Serial.print(error);
    Serial.print(" Cor: ");
    Serial.println(correction);
}

// ─── RESET PID ───────────────────────────────────────────────
// Call this when robot stops or changes behavior
void resetPID()
{
    lastError = 0;
    integral = 0;
}