#pragma once
#include <Arduino.h>
#include <QTRSensors.h>

// ─── CONFIG ──────────────────────────────────────────────────
#define NUM_SENSORS 8
#define EMITTER_PIN 2 // controls IR emitters ON/OFF

// ─── IR WALL SENSORS ─────────────────────────────────────────
#define LEFT_IR 14  // left wall sensor
#define RIGHT_IR 42 // right wall sensor

// ─── SENSOR PINS ─────────────────────────────────────────────
const uint8_t SENSOR_PINS[NUM_SENSORS] = {
    40, 21, 41, 45, 16, 18, 3, 46
    //  S1  S2  S3 S4  S5  S6  S7  S8
    //  LEFT                      RIGHT
};

// ─── GLOBALS ─────────────────────────────────────────────────
QTRSensors qtr;
uint16_t sensorValues[NUM_SENSORS]; // raw readings per sensor
int16_t position;                   // 0 to 7000, center = 3500

// ─────────────────────────────────────────────────────────────
void sensorInit()
{
    // QTR line sensors
    qtr.setTypeRC();
    qtr.setSensorPins(SENSOR_PINS, NUM_SENSORS);
    qtr.setEmitterPin(EMITTER_PIN);

    // IR wall sensors
    pinMode(LEFT_IR, INPUT);
    pinMode(RIGHT_IR, INPUT);
}

// ─── CALIBRATION ─────────────────────────────────────────────
// Call this once at startup — robot must sweep over the line
void calibrateSensors()
{
    Serial.println("Calibrating... move robot over line");

    qtr.emittersOn();

    for (int i = 0; i < 200; i++)
    {
        qtr.calibrate();
        delay(20);
    }

    qtr.emittersOff();
    Serial.println("Calibration done!");

    // Print calibration results for debugging
    Serial.println("Min values:");
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        Serial.print(qtr.calibrationOn.minimum[i]);
        Serial.print(" ");
    }
    Serial.println();

    Serial.println("Max values:");
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        Serial.print(qtr.calibrationOn.maximum[i]);
        Serial.print(" ");
    }
    Serial.println();
}

// ─── READ POSITION ───────────────────────────────────────────
// Returns position: 0 (far left) → 3500 (center) → 7000 (far right)
int16_t readLine()
{
    position = qtr.readLineBlack(sensorValues);
    return position;
}

// ─── HELPER: IS LINE LOST? ───────────────────────────────────
bool isLineLost()
{
    readLine();
    int activeCount = 0;
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > 500)
            activeCount++;
    }
    return (activeCount == 0);
}

// ─── HELPER: INTERSECTION DETECTION ─────────────────────────
// Returns true if robot hits a T or cross intersection
bool isIntersection()
{
    readLine();
    return (sensorValues[0] > 500 && sensorValues[7] > 500);
}

// ─── HELPER: DEAD END ────────────────────────────────────────
bool isDeadEnd()
{
    readLine();
    int activeCount = 0;
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > 500)
            activeCount++;
    }
    return (activeCount <= 2 && sensorValues[0] < 500 && sensorValues[7] < 500);
}

// ─── WALL DETECTION ──────────────────────────────────────────
// IR sensors: LOW = wall detected, HIGH = no wall
bool wallLeft()
{
    return (digitalRead(LEFT_IR) == LOW);
}

bool wallRight()
{
    return (digitalRead(RIGHT_IR) == LOW);
}