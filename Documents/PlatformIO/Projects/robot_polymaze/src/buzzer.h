#pragma once
#include <Arduino.h>
#include "sensor.h"

// ─── BUZZER PIN ──────────────────────────────────────────────
#define BUZZER_PIN 13

// ─── WHISTLE CONFIG ──────────────────────────────────────────
// Football final whistle = 3 beeps with specific tempo
#define WHISTLE_FREQ 2000    // Hz — pitch of whistle sound
#define WHISTLE_DURATION 300 // ms — how long each beep lasts
#define WHISTLE_PAUSE 200    // ms — pause between beeps
#define WHISTLE_COUNT 3      // 3 whistles = end of match

// ─────────────────────────────────────────────────────────────
void buzzerInit()
{
    // Configure LEDC channel 2 for buzzer
    // (channels 0,1 already used by motors)
    ledcSetup(2, WHISTLE_FREQ, 8);
    ledcAttachPin(BUZZER_PIN, 2);
    ledcWrite(2, 0); // silent at start
}

// ─── SINGLE BEEP ─────────────────────────────────────────────
void beep(int frequency, int duration)
{
    ledcSetup(2, frequency, 8);
    ledcWrite(2, 128); // 50% duty cycle = loudest
    delay(duration);
    ledcWrite(2, 0); // silent
    delay(WHISTLE_PAUSE);
}

// ─── FINAL WHISTLE ───────────────────────────────────────────
// 3 whistles respecting football match tempo
// Called when robot reaches finish line (black square)
void finalWhistle()
{
    Serial.println("FINAL WHISTLE!");

    // First whistle — short
    beep(WHISTLE_FREQ, WHISTLE_DURATION);

    // Second whistle — short
    beep(WHISTLE_FREQ, WHISTLE_DURATION);

    // Third whistle — long (held longer = final whistle feel)
    beep(WHISTLE_FREQ, WHISTLE_DURATION * 2);
}

// ─── FINISH LINE DETECTION ───────────────────────────────────
// Finish line = 20x20cm black square (from POLYMAZE spec)
// All 8 sensors see black = robot is on finish square
bool isFinishLine()
{
    readLine();
    int activeCount = 0;
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > 500)
            activeCount++;
    }
    // 6+ sensors on black = finish square detected
    return (activeCount >= 6);
}