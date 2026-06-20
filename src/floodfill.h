#pragma once
#include <Arduino.h>
#include "intersection.h"
#include "sensor.h"
#include "maze.h"

// ─── FLOODFILL CONFIG ────────────────────────────────────────
#define MAZE_SIZE 11         // Updated for 11x11 grid
#define TARGET_X 5           // Center of 11x11 is (5,5)
#define TARGET_Y 5
#define MAX_CELLS (MAZE_SIZE * MAZE_SIZE)

// ─── WALL BITMASKS ───────────────────────────────────────────
#define BIT_N 0x01
#define BIT_E 0x02
#define BIT_S 0x04
#define BIT_W 0x08

// ─── DIRECTION HEADINGS ──────────────────────────────────────
enum Heading {
    HEAD_N = 0,
    HEAD_E = 1,
    HEAD_S = 2,
    HEAD_W = 3
};

// ─── FLOODFILL STATE GLOBALS ─────────────────────────────────
uint8_t walls[MAZE_SIZE][MAZE_SIZE];
uint8_t dist[MAZE_SIZE][MAZE_SIZE];

int curX = 0;
int curY = 0;
Heading curHeading = HEAD_N;

// ─────────────────────────────────────────────────────────────
void floodfillInit() {
    curX = 0;
    curY = 0;
    curHeading = HEAD_N;

    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int y = 0; y < MAZE_SIZE; y++) {
            walls[x][y] = 0; 
            if (y == MAZE_SIZE - 1) walls[x][y] |= BIT_N;
            if (x == MAZE_SIZE - 1) walls[x][y] |= BIT_E;
            if (y == 0)             walls[x][y] |= BIT_S;
            if (x == 0)             walls[x][y] |= BIT_W;
        }
    }
}

// ─── VIRTUAL ODOMETRY SUPPORT ────────────────────────────────
// Call this exactly ONCE from your main loop when the robot 
// covers 30cm (either via physical line or MPU distance)
void advancePosition() {
    if (curHeading == HEAD_N && curY < MAZE_SIZE - 1) curY++;
    else if (curHeading == HEAD_E && curX < MAZE_SIZE - 1) curX++;
    else if (curHeading == HEAD_S && curY > 0) curY--;
    else if (curHeading == HEAD_W && curX > 0) curX--;
    
    Serial.print("Moved to X:"); Serial.print(curX);
    Serial.print(" Y:"); Serial.println(curY);
}

// Call this AFTER the robot physically completes a turn
// to keep the software compass synced with reality
void updateHeading(Decision d) {
    if (d == D_LEFT) {
        curHeading = (Heading)((curHeading + 3) % 4);
    } else if (d == D_RIGHT) {
        curHeading = (Heading)((curHeading + 1) % 4);
    } else if (d == D_UTURN) {
        curHeading = (Heading)((curHeading + 2) % 4);
    }
    // D_STRAIGHT does nothing to heading
}

// ─── WALL DISCOVERY ──────────────────────────────────────────
void updateWalls(bool wLeft, bool wRight, bool wFront) {
    uint8_t absoluteFront = 0, absoluteLeft = 0, absoluteRight = 0;

    switch (curHeading) {
        case HEAD_N: absoluteFront = BIT_N; absoluteLeft = BIT_W; absoluteRight = BIT_E; break;
        case HEAD_E: absoluteFront = BIT_E; absoluteLeft = BIT_N; absoluteRight = BIT_S; break;
        case HEAD_S: absoluteFront = BIT_S; absoluteLeft = BIT_E; absoluteRight = BIT_W; break;
        case HEAD_W: absoluteFront = BIT_W; absoluteLeft = BIT_S; absoluteRight = BIT_N; break;
    }

    if (wFront) walls[curX][curY] |= absoluteFront;
    if (wLeft)  walls[curX][curY] |= absoluteLeft;
    if (wRight) walls[curX][curY] |= absoluteRight;

    // Project discovered walls to adjacent neighboring cells
    if (wFront && curHeading == HEAD_N && curY < MAZE_SIZE - 1) walls[curX][curY + 1] |= BIT_S;
    if (wFront && curHeading == HEAD_E && curX < MAZE_SIZE - 1) walls[curX + 1][curY] |= BIT_W;
    if (wFront && curHeading == HEAD_S && curY > 0)             walls[curX][curY - 1] |= BIT_N;
    if (wFront && curHeading == HEAD_W && curX > 0)             walls[curX - 1][curY] |= BIT_E;

    if (wLeft && curHeading == HEAD_N && curX > 0)             walls[curX - 1][curY] |= BIT_E;
    if (wLeft && curHeading == HEAD_E && curY < MAZE_SIZE - 1) walls[curX][curY + 1] |= BIT_S;
    if (wLeft && curHeading == HEAD_S && curX < MAZE_SIZE - 1) walls[curX + 1][curY] |= BIT_W;
    if (wLeft && curHeading == HEAD_W && curY > 0)             walls[curX][curY - 1] |= BIT_N;

    if (wRight && curHeading == HEAD_N && curX < MAZE_SIZE - 1) walls[curX + 1][curY] |= BIT_W;
    if (wRight && curHeading == HEAD_E && curY > 0)             walls[curX][curY - 1] |= BIT_N;
    if (wRight && curHeading == HEAD_S && curX > 0)             walls[curX - 1][curY] |= BIT_E;
    if (wRight && curHeading == HEAD_W && curY < MAZE_SIZE - 1) walls[curX][curY + 1] |= BIT_S;
}

// ─── BREADTH-FIRST SEARCH ────────────────────────────────────
void runFloodfill() {
    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int y = 0; y < MAZE_SIZE; y++) {
            dist[x][y] = 255; 
        }
    }

    int queueX[MAX_CELLS];
    int queueY[MAX_CELLS];
    int head = 0, tail = 0;

    dist[TARGET_X][TARGET_Y] = 0;
    queueX[tail] = TARGET_X; queueY[tail] = TARGET_Y; tail++;

    while (head < tail) {
        int x = queueX[head]; int y = queueY[head]; head++;
        uint8_t currentDistance = dist[x][y];
        uint8_t cellWalls = walls[x][y];

        if (!(cellWalls & BIT_N) && y < MAZE_SIZE - 1 && dist[x][y + 1] == 255) {
            dist[x][y + 1] = currentDistance + 1;
            queueX[tail] = x; queueY[tail] = y + 1; tail++;
        }
        if (!(cellWalls & BIT_E) && x < MAZE_SIZE - 1 && dist[x + 1][y] == 255) {
            dist[x + 1][y] = currentDistance + 1;
            queueX[tail] = x + 1; queueY[tail] = y; tail++;
        }
        if (!(cellWalls & BIT_S) && y > 0 && dist[x][y - 1] == 255) {
            dist[x][y - 1] = currentDistance + 1;
            queueX[tail] = x; queueY[tail] = y - 1; tail++;
        }
        if (!(cellWalls & BIT_W) && x > 0 && dist[x - 1][y] == 255) {
            dist[x - 1][y] = currentDistance + 1;
            queueX[tail] = x - 1; queueY[tail] = y; tail++;
        }
    }
}

// ─── DECIDE NEXT MOVE ────────────────────────────────────────
// Now ONLY analyzes paths and returns a decision. 
// It does NOT update coordinates or heading.
Decision getFloodfillDecision() {
    uint8_t cellWalls = walls[curX][curY];
    uint8_t minCalculatedDist = 255;
    Heading targetHeading = curHeading;

    if (!(cellWalls & BIT_N) && curY < MAZE_SIZE - 1 && dist[curX][curY + 1] < minCalculatedDist) {
        minCalculatedDist = dist[curX][curY + 1]; targetHeading = HEAD_N;
    }
    if (!(cellWalls & BIT_E) && curX < MAZE_SIZE - 1 && dist[curX + 1][curY] < minCalculatedDist) {
        minCalculatedDist = dist[curX + 1][curY]; targetHeading = HEAD_E;
    }
    if (!(cellWalls & BIT_S) && curY > 0 && dist[curX][curY - 1] < minCalculatedDist) {
        minCalculatedDist = dist[curX][curY - 1]; targetHeading = HEAD_S;
    }
    if (!(cellWalls & BIT_W) && curX > 0 && dist[curX - 1][curY] < minCalculatedDist) {
        minCalculatedDist = dist[curX - 1][curY]; targetHeading = HEAD_W;
    }

    int headingDifference = (targetHeading - curHeading + 4) % 4;
    
    switch (headingDifference) {
        case 0: return D_STRAIGHT;
        case 1: return D_RIGHT;
        case 2: return D_UTURN;
        case 3: return D_LEFT;
    }
    return D_STRAIGHT;
}