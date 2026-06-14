#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ─── COLOR THRESHOLDS ────────────────────────────────────────
// These are starting values — calibrate after testing!
// ─── COLOR THRESHOLDS ────────────────────────────────────────
// Based on actual calibration data

// BLACK — C value below this = black
#define BLACK_C_MAX 250 // your black C was 155-188

// WHITE — C value above this + R highest
#define WHITE_C_MIN 800

// GREEN — G and B both high, C high
#define GREEN_G_MIN 70  // gNorm > 85
#define GREEN_B_MIN 70  // bNorm > 85
#define GREEN_C_MIN 800 // C > 1200

// RED — need to retest with true red surface
// temporary thresholds until you retest
#define RED_R_MIN 100 // rNorm > 100
#define RED_G_MAX 150 // gNorm < 150

// ─── LED PINS ────────────────────────────────────────────────
#define RED_LED_PIN 9
#define GREEN_LED_PIN 8

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
    Wire.begin(21, 20);
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

// ─── CONFIRMATION FILTER ─────────────────────────────────────
#define CONFIRM_COUNT 5 // must see same color N times in a row

DetectedColor lastColor = COLOR_NONE;
int colorCount = 0;

DetectedColor getRawColor()
{
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);

    if (c == 0)
        return COLOR_NONE;

    float rN = (float)r / c * 255;
    float gN = (float)g / c * 255;
    float bN = (float)b / c * 255;

    Serial.print("R:");
    Serial.print(rN);
    Serial.print(" G:");
    Serial.print(gN);
    Serial.print(" B:");
    Serial.print(bN);
    Serial.print(" C:");
    Serial.print(c);
    Serial.print(" G/R:");
    Serial.println(gN / rN);

    // BLACK — very low total light
    if (c < 250)
        return COLOR_BLACK;

    // RED — R clearly dominant
    if (rN > gN * 1.3 && rN > bN * 1.3 && rN > 80)
    {
        return COLOR_RED;
    }

    // GREEN — high G/R ratio AND medium-low C
    // AIR has same G/R but much higher C → excluded
    if (gN / rN > 1.7 && c < 1000)
    {
        return COLOR_GREEN;
    }

    // WHITE — R dominant, high C
    if (c > 800 && rN > gN && rN > bN)
    {
        return COLOR_WHITE;
    }

    return COLOR_NONE;
}

DetectedColor detectColor()
{
    DetectedColor current = getRawColor();

    if (current == lastColor)
    {
        colorCount++;
    }
    else
    {
        // color changed → reset counter
        colorCount = 1;
        lastColor = current;
    }

    // only return confirmed color
    if (colorCount >= CONFIRM_COUNT)
    {
        return current;
    }

    // not confirmed yet → return NONE
    return COLOR_NONE;
}
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