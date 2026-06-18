#pragma once
#include <Arduino.h>
#include "driver/ledc.h"

// ─── PIN DEFINITIONS ─────────────────────────────────────────
// Change these to match your actual wiring later
#define AIN1 10
#define AIN2 9
#define PWMA 46
#define BIN1 12
#define BIN2 13
#define PWMB 14
#define STBY 11

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
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(STBY, OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = LEDC_FREQ,
        .clk_cfg = LEDC_AUTO_CLK};
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_A = {
        .gpio_num = PWMA,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel_A);

    ledc_channel_config_t ledc_channel_B = {
        .gpio_num = PWMB,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0};
    ledc_channel_config(&ledc_channel_B);

    digitalWrite(STBY, HIGH);
}

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

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
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
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
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