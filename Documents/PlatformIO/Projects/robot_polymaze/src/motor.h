#pragma once
#include <Arduino.h>

// ─── PIN DEFINITIONS ─────────────────────────────────────────
// Change these to match your actual wiring later
#define AIN1 27
#define AIN2 26
#define PWMA 25 // Motor A (LEFT)

#define BIN1 33
#define BIN2 32
#define PWMB 14 // Motor B (RIGHT)

#define STBY 12 // Standby pin — must be HIGH to enable driver

// ─── PWM CONFIG (ESP32 LEDC) ─────────────────────────────────
#define LEDC_FREQ 1000 // 1kHz PWM frequency
#define LEDC_RES 8     // 8-bit resolution → values 0–255
#define LEDC_CH_A 0    // LEDC channel for motor A
#define LEDC_CH_B 1    // LEDC channel for motor B

// ─── SPEED LIMITS ────────────────────────────────────────────
#define MAX_SPEED 200  // out of 255 — safety cap for now
#define BASE_SPEED 120 // default cruising speed

// ─────────────────────────────────────────────────────────────
void motorsInit()
{
    // Direction pins
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(STBY, OUTPUT);

    // Setup LEDC PWM channels
    ledcSetup(LEDC_CH_A, LEDC_FREQ, LEDC_RES);
    ledcSetup(LEDC_CH_B, LEDC_FREQ, LEDC_RES);
    ledcAttachPin(PWMA, LEDC_CH_A);
    ledcAttachPin(PWMB, LEDC_CH_B);

    // Enable driver
    digitalWrite(STBY, HIGH);
}

// ─── SET INDIVIDUAL MOTOR SPEED ──────────────────────────────
// speed: -255 (full back) to +255 (full forward)
void setMotorA(int speed)
{
    speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
    if (speed > 0)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
    }
    else if (speed < 0)
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        speed = -speed;
    }
    else
    {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, LOW);
    }
    ledcWrite(LEDC_CH_A, speed);
}

void setMotorB(int speed)
{
    speed = constrain(speed, -MAX_SPEED, MAX_SPEED);
    if (speed > 0)
    {
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    }
    else if (speed < 0)
    {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
        speed = -speed;
    }
    else
    {
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, LOW);
    }
    ledcWrite(LEDC_CH_B, speed);
}

// ─── HIGH LEVEL DRIVE FUNCTIONS ──────────────────────────────
void driveForward(int speed)
{
    setMotorA(speed);
    setMotorB(speed);
}

void stopMotors()
{
    setMotorA(0);
    setMotorB(0);
}

void turnLeft(int speed)
{
    setMotorA(-speed); // left motor backward
    setMotorB(speed);  // right motor forward
}

void turnRight(int speed)
{
    setMotorA(speed);  // left motor forward
    setMotorB(-speed); // right motor backward
}