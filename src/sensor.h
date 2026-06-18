#pragma once
#include <Arduino.h>
#include <QTRSensors.h>

#define NUM_SENSORS 8
#define EMITTER_PIN 8

const uint8_t SENSOR_PINS[NUM_SENSORS] = {
    18, 17, 16, 15, 7, 6, 5, 4

};

#define LINE_DETECT_THRESHOLD 500
#define LINE_DETECT_MIN_ACTIVE 1

QTRSensors qtr;
uint16_t sensorValues[NUM_SENSORS];
int16_t position;

void sensorInit()
{
    qtr.setTypeRC();
    qtr.setSensorPins(SENSOR_PINS, NUM_SENSORS);
    qtr.setEmitterPin(EMITTER_PIN);
}

void calibrateSensors()
{
    Serial.println("Calibrating, move robot over line");

    qtr.emittersOn();

    for (int i = 0; i < 200; i++)
    {
        qtr.calibrate();
        delay(20);
    }

    qtr.emittersOff();
    Serial.println("Calibration done");
}

int16_t readLine()
{
    position = qtr.readLineBlack(sensorValues);
    return position;
}

bool isLineDetected()
{
    readLine();
    int activeCount = 0;

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > LINE_DETECT_THRESHOLD)
            activeCount++;
    }

    return (activeCount >= LINE_DETECT_MIN_ACTIVE);
}

bool isLineLost()
{
    return !isLineDetected();
}

bool isIntersection()
{
    readLine();
    return (sensorValues[0] > LINE_DETECT_THRESHOLD && sensorValues[7] > LINE_DETECT_THRESHOLD);
}

bool isDeadEnd()
{
    readLine();
    int activeCount = 0;
    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (sensorValues[i] > LINE_DETECT_THRESHOLD)
            activeCount++;
    }
    return (activeCount <= 2 && sensorValues[0] < LINE_DETECT_THRESHOLD && sensorValues[7] < LINE_DETECT_THRESHOLD);
}