#pragma once
#include <Arduino.h>
#include "motor.h"
#include "sensor.h"
#include "pid.h"
#include "intersection.h"

// ─── MAZE CONFIG ─────────────────────────────────────────────
#define MAX_PATH 500 // max intersections to remember
#define SWITCH_PIN 0 // GPIO0 — use your interrupteur here

// ─── HAND RULE SELECTION ─────────────────────────────────────
// Will be controlled by second switch later
bool useLeftHand = true; // true=left hand, false=right hand

// ─── SPEED RUN CONFIG ────────────────────────────────────────
#define EXPLORE_SPEED 110 // slower = safer during exploration
#define SPRINT_SPEED 180  // faster = better score on speed run

// ─── MEMORY ──────────────────────────────────────────────────
// Stores every decision made at each intersection
enum Decision
{
    D_LEFT,
    D_RIGHT,
    D_STRAIGHT,
    D_UTURN
};

Decision pathMemory[MAX_PATH]; // sequence of decisions
int pathLength = 0;            // how many decisions stored
int pathIndex = 0;             // current position in replay
bool mazeExplored = false;     // has robot reached finish?
bool speedRunMode = false;     // is robot in speed run?

// ─────────────────────────────────────────────────────────────
void mazeInit()
{
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    pathLength = 0;
    pathIndex = 0;
    mazeExplored = false;

    // Read switch position at startup
    speedRunMode = (digitalRead(SWITCH_PIN) == LOW);

    Serial.print("Mode: ");
    Serial.println(speedRunMode ? "SPEED RUN" : "EXPLORE");
}

// ─── SAVE DECISION ───────────────────────────────────────────
void saveDecision(Decision d)
{
    if (pathLength < MAX_PATH)
    {
        pathMemory[pathLength++] = d;
        Serial.print("Saved: ");
        Serial.println(d);
    }
}

// ─── LEFT HAND RULE DECISION ─────────────────────────────────
// Given intersection type → decide which way to go
// Left hand rule: prefer left → straight → right → uturn
Decision leftHandDecision(IntersectionType type)
{
    switch (type)
    {
    case TURN_LEFT:
        return D_LEFT;
    case T_LEFT:
        return D_LEFT; // left available → take it
    case CROSS:
        return D_LEFT; // left always preferred
    case T_RIGHT:
        return D_STRAIGHT; // no left → go straight
    case TURN_RIGHT:
        return D_RIGHT; // forced right
    case DEAD_END:
        return D_UTURN; // dead end → turn back
    default:
        return D_STRAIGHT;
    }
}

// ─── RIGHT HAND RULE DECISION ────────────────────────────────
// Mirror of left hand — prefer right → straight → left → uturn
Decision rightHandDecision(IntersectionType type)
{
    switch (type)
    {
    case TURN_RIGHT:
        return D_RIGHT;
    case T_RIGHT:
        return D_RIGHT;
    case CROSS:
        return D_RIGHT;
    case T_LEFT:
        return D_STRAIGHT;
    case TURN_LEFT:
        return D_LEFT;
    case DEAD_END:
        return D_UTURN;
    default:
        return D_STRAIGHT;
    }
}

// ─── EXECUTE DECISION ────────────────────────────────────────
void executeDecision(Decision d)
{
    switch (d)
    {
    case D_LEFT:
        doTurnLeft();
        break;
    case D_RIGHT:
        doTurnRight();
        break;
    case D_STRAIGHT:
        goStraightThrough();
        break;
    case D_UTURN:
        doUTurn();
        break;
    }
}

// ─── EXPLORATION STEP ────────────────────────────────────────
// Called at every intersection during run 1
void exploreStep(IntersectionType type)
{
    // Choose algorithm based on switch
    Decision d = useLeftHand
                     ? leftHandDecision(type)
                     : rightHandDecision(type);

    // Save decision to memory
    saveDecision(d);

    // Execute it
    executeDecision(d);
}

// ─── SPEED RUN STEP ──────────────────────────────────────────
// Called at every intersection during run 2
// Just replays memorized decisions — no thinking needed
void speedRunStep()
{
    if (pathIndex < pathLength)
    {
        Decision d = pathMemory[pathIndex++];
        executeDecision(d);
        Serial.print("Replaying: ");
        Serial.println(pathIndex);
    }
}