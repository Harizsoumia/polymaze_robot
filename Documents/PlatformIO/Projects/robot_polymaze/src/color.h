#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ─── COLOR THRESHOLDS ────────────────────────────────────────
// These are starting values — calibrate after testing!
#define RED_R_MIN 150
#define RED_G_MAX 100
#define RED_B_MAX 100

#define GREEN_R_MAX 100
#define GREEN_G_MIN 150
#define GREEN_B_MAX 100

// ─── LED PINS ────────────────────────────────────────────────
#define RED_LED_PIN 23
#define GREEN_LED_PIN 12

// ─── DETECTED COLOR ──────────────────────────────────────────
enum DetectedColor
{
    COLOR_NONE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_WHITE,
    COLOR_BLACK
};

// ─── SENSOR OBJECT ───────────────────────────────────────────
// Integration time + gain affect sensitivity
// TCS34725_INTEGRATIONTIME_50MS = good balance for speed
// TCS34725_GAIN_4X = good for normal lighting
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_50MS,
    TCS34725_GAIN_4X);

// ─────────────────────────────────────────────────────────────
void colorInit()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);

    // Turn both LEDs off at start
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);

    if (!tcs.begin())
    {
        Serial.println("TCS34725 not found! Check wiring.");
        while (1)
            ; // stop everything — sensor is critical
    }
    Serial.println("TCS34725 ready!");
}

// ─── READ RAW RGB ────────────────────────────────────────────
void readRGB(uint16_t &r, uint16_t &g, uint16_t &b, uint16_t &c)
{
    tcs.getRawData(&r, &g, &b, &c);
}

// ─── DETECT COLOR ────────────────────────────────────────────
DetectedColor detectColor()
{
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

    // Normalize by clear channel to handle lighting changes
    if (c == 0)
        return COLOR_NONE;

    float rNorm = (float)r / c * 255;
    float gNorm = (float)g / c * 255;
    float bNorm = (float)b / c * 255;

    Serial.print("R:");
    Serial.print(rNorm);
    Serial.print(" G:");
    Serial.print(gNorm);
    Serial.print(" B:");
    Serial.println(bNorm);

    // Red band detection
    if (rNorm > RED_R_MIN && gNorm < RED_G_MAX && bNorm < RED_B_MAX)
    {
        return COLOR_RED;
    }

    // Green band detection
    if (rNorm < GREEN_R_MAX && gNorm > GREEN_G_MIN && bNorm < GREEN_B_MAX)
    {
        return COLOR_GREEN;
    }

    // Black line
    if (c < 500)
    {
        return COLOR_BLACK;
    }

    // White surface
    if (c > 2000)
    {
        return COLOR_WHITE;
    }

    return COLOR_NONE;
}

// ─── CHECKPOINT RESPONSE ─────────────────────────────────────
// Called when checkpoint detected — activates correct LED
void handleCheckpoint(DetectedColor color)
{
    if (color == COLOR_RED)
    {
        Serial.println("RED checkpoint detected!");
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        delay(500); // hold LED for 500ms
        digitalWrite(RED_LED_PIN, LOW);
    }
    else if (color == COLOR_GREEN)
    {
        Serial.println("GREEN checkpoint detected!");
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
        delay(500);
        digitalWrite(GREEN_LED_PIN, LOW);
    }
}

// ─── CALIBRATION HELPER ──────────────────────────────────────
// Run this to find correct threshold values for your environment
void calibrateColor()
{
    Serial.println("Place sensor over RED band...");
    delay(3000);
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    float rN = (float)r / c * 255;
    float gN = (float)g / c * 255;
    float bN = (float)b / c * 255;
    Serial.print("RED → R:");
    Serial.print(rN);
    Serial.print(" G:");
    Serial.print(gN);
    Serial.print(" B:");
    Serial.println(bN);

    Serial.println("Place sensor over GREEN band...");
    delay(3000);
    tcs.getRawData(&r, &g, &b, &c);
    rN = (float)r / c * 255;
    gN = (float)g / c * 255;
    bN = (float)b / c * 255;
    Serial.print("GREEN → R:");
    Serial.print(rN);
    Serial.print(" G:");
    Serial.print(gN);
    Serial.print(" B:");
    Serial.println(bN);
}