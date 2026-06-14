//for testing purposes for now THIS IS ONLY A TEMPLATE
#pragma once
#include <Arduino.h>
#include "intersection.h"
#include "sensor.h"
#include "maze.h"

// ─── FLOODFILL CONFIG ────────────────────────────────────────
#define MAZE_SIZE 8          // Change to 16 for a 16x16 maze
#define TARGET_X 3           // Center/destination coordinates
#define TARGET_Y 3           // (For 2x2 centers, adapt to match target)
#define MAX_CELLS (MAZE_SIZE * MAZE_SIZE)

// ─── WALL BITMASKS ───────────────────────────────────────────
#define BIT_N 0x01 // 0001
#define BIT_E 0x02 // 0010
#define BIT_S 0x04 // 0100
#define BIT_W 0x08 // 1000

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
Heading curHeading = HEAD_N; // Assuming robot starts facing North

// ─────────────────────────────────────────────────────────────
// Initialize or reset map configurations
void floodfillInit() {
    curX = 0;
    curY = 0;
    curHeading = HEAD_N;

    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int y = 0; y < MAZE_SIZE; y++) {
            walls[x][y] = 0; // Clear all discovered internal walls
            
            // Pre-load outer boundary walls of the maze perimeter
            if (y == MAZE_SIZE - 1) walls[x][y] |= BIT_N;
            if (x == MAZE_SIZE - 1) walls[x][y] |= BIT_E;
            if (y == 0)             walls[x][y] |= BIT_S;
            if (x == 0)             walls[x][y] |= BIT_W;
        }
    }
}

// ─── WALL DISCOVERY & BIT CONVERSIONS ─────────────────────────
// Updates the coordinate map relative to the global compass heading
void updateWalls(bool wLeft, bool wRight, bool wFront) {
    uint8_t absoluteFront = 0;
    uint8_t absoluteLeft = 0;
    uint8_t absoluteRight = 0;

    // Map relative sensor arrays to absolute cardinal directions
    switch (curHeading) {
        case HEAD_N:
            absoluteFront = BIT_N; absoluteLeft = BIT_W; absoluteRight = BIT_E;
            break;
        case HEAD_E:
            absoluteFront = BIT_E; absoluteLeft = BIT_N; absoluteRight = BIT_S;
            break;
        case HEAD_S:
            absoluteFront = BIT_S; absoluteLeft = BIT_E; absoluteRight = BIT_W;
            break;
        case HEAD_W:
            absoluteFront = BIT_W; absoluteLeft = BIT_S; absoluteRight = BIT_N;
            break;
    }

    // Apply readings to the current cell bitmask
    if (wFront) walls[curX][curY] |= absoluteFront;
    if (wLeft)  walls[curX][curY] |= absoluteLeft;
    if (wRight) walls[curX][curY] |= absoluteRight;

    // Project discovered walls to adjacent neighboring cells to prevent mismatch
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

// ─── RE-FLOOD DISTANCE MATRIX ────────────────────────────────
// Standard Breadth-First Search (BFS) starting from target cell(s)
void runFloodfill() {
    // Reset all distances to an unvisited maximum
    for (int x = 0; x < MAZE_SIZE; x++) {
        for (int y = 0; y < MAZE_SIZE; y++) {
            dist[x][y] = 255; 
        }
    }

    // Array-based simple FIFO queue to avoid memory fragmentation on MCU
    int queueX[MAX_CELLS];
    int queueY[MAX_CELLS];
    int head = 0, tail = 0;

    // Seed the target cell
    dist[TARGET_X][TARGET_Y] = 0;
    queueX[tail] = TARGET_X;
    queueY[tail] = TARGET_Y;
    tail++;

    // Flood algorithm loop
    while (head < tail) {
        int x = queueX[head];
        int y = queueY[head];
        head++;

        uint8_t currentDistance = dist[x][y];
        uint8_t cellWalls = walls[x][y];

        // Check North Neighbor
        if (!(cellWalls & BIT_N) && y < MAZE_SIZE - 1 && dist[x][y + 1] == 255) {
            dist[x][y + 1] = currentDistance + 1;
            queueX[tail] = x; queueY[tail] = y + 1; tail++;
        }
        // Check East Neighbor
        if (!(cellWalls & BIT_E) && x < MAZE_SIZE - 1 && dist[x + 1][y] == 255) {
            dist[x + 1][y] = currentDistance + 1;
            queueX[tail] = x + 1; queueY[tail] = y; tail++;
        }
        // Check South Neighbor
        if (!(cellWalls & BIT_S) && y > 0 && dist[x][y - 1] == 255) {
            dist[x][y - 1] = currentDistance + 1;
            queueX[tail] = x; queueY[tail] = y - 1; tail++;
        }
        // Check West Neighbor
        if (!(cellWalls & BIT_W) && x > 0 && dist[x - 1][y] == 255) {
            dist[x - 1][y] = currentDistance + 1;
            queueX[tail] = x - 1; queueY[tail] = y; tail++;
        }
    }
}

// ─── DECIDE NEXT EXECUTION MOVE ──────────────────────────────
// Analyzes open neighbors and picks direction with lowest step values
Decision getFloodfillDecision() {
    uint8_t cellWalls = walls[curX][curY];
    uint8_t minCalculatedDist = 255;
    Heading targetHeading = curHeading;

    // Check available directions around the current coordinate cell using curX and curY
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

    // Determine target execution action relative to current orientation
    int headingDifference = (targetHeading - curHeading + 4) % 4;
    
    // Update heading and track internal software coordinates ahead of physical loop execution
    curHeading = targetHeading;
    switch (headingDifference) {
        case 0:  
            if (curHeading == HEAD_N) curY++;
            else if (curHeading == HEAD_E) curX++;
            else if (curHeading == HEAD_S) curY--;
            else if (curHeading == HEAD_W) curX--;
            return D_STRAIGHT;
        case 1:  
            if (curHeading == HEAD_N) curY++;
            else if (curHeading == HEAD_E) curX++;
            else if (curHeading == HEAD_S) curY--;
            else if (curHeading == HEAD_W) curX--;
            return D_RIGHT;
        case 2:  
            if (curHeading == HEAD_N) curY++;
            else if (curHeading == HEAD_E) curX++;
            else if (curHeading == HEAD_S) curY--;
            else if (curHeading == HEAD_W) curX--;
            return D_UTURN;
        case 3:  
            if (curHeading == HEAD_N) curY++;
            else if (curHeading == HEAD_E) curX++;
            else if (curHeading == HEAD_S) curY--;
            else if (curHeading == HEAD_W) curX--;
            return D_LEFT;
    }
    return D_STRAIGHT;
}