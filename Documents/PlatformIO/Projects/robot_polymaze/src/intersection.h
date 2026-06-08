#pragma once
#include <Arduino.h>
#include "motor.h"
#include "sensor.h"
#include "pid.h"

// ─── INTERSECTION TYPES ──────────────────────────────────────
enum IntersectionType
{
    NONE,
    TURN_LEFT,
    TURN_RIGHT,
    T_LEFT,  // T intersection, path goes left + straight
    T_RIGHT, // T intersection, path goes right + straight
    CROSS,   // 4 way intersection
    DEAD_END
};

// ─── TURN SPEEDS ─────────────────────────────────────────────
#define TURN_SPEED 120
#define TURN_DURATION 300 // ms — adjust after testing

// ─────────────────────────────────────────────────────────────
// Detect what kind of intersection robot is at
IntersectionType detectIntersection()
{
    readLine();

    bool leftEdge = (sensorValues[0] > 500);  // S1 sees line
    bool rightEdge = (sensorValues[7] > 500); // S8 sees line
    bool center = (sensorValues[3] > 500 || sensorValues[4] > 500);
    bool wallL = wallLeft();
    bool wallR = wallRight();

    // Dead end — very few sensors active
    if (isDeadEnd())
    {
        return DEAD_END;
    }

    // Cross — both edges + center active + no walls both sides
    if (leftEdge && rightEdge && center)
    {
        return CROSS;
    }

    // T left — left edge + center, no right path
    if (leftEdge && center && !rightEdge && !wallL)
    {
        return T_LEFT;
    }

    // T right — right edge + center, no left path
    if (rightEdge && center && !leftEdge && !wallR)
    {
        return T_RIGHT;
    }

    // Simple left turn — only left edge
    if (leftEdge && !rightEdge && !center)
    {
        return TURN_LEFT;
    }

    // Simple right turn — only right edge
    if (rightEdge && !leftEdge && !center)
    {
        return TURN_RIGHT;
    }

    return NONE;
}

// ─── TURN ACTIONS ────────────────────────────────────────────
void doTurnLeft()
{
    resetPID();
    // First move forward a little to center on intersection
    driveForward(BASE_SPEED);
    delay(100);
    // Then turn
    turnLeft(TURN_SPEED);
    delay(TURN_DURATION);
    stopMotors();
    delay(50);
}

void doTurnRight()
{
    resetPID();
    driveForward(BASE_SPEED);
    delay(100);
    turnRight(TURN_SPEED);
    delay(TURN_DURATION);
    stopMotors();
    delay(50);
}

void doUTurn()
{
    resetPID();
    turnRight(TURN_SPEED);
    delay(TURN_DURATION * 2); // double time = 180°
    stopMotors();
    delay(50);
}

void goStraightThrough()
{
    resetPID();
    driveForward(BASE_SPEED);
    delay(200); // push through intersection
}