#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>
#include <TCA9548.h>

// ─── WHEEL CONFIG ────────────────────────────────────────────
#define WHEEL_DIAMETER_MM 60.0                       // ← change after measuring!
#define WHEEL_CIRCUMFERENCE (PI * WHEEL_DIAMETER_MM) // ~188.5mm

// POLYMAZE maze cell = 30cm × 30cm
#define CELL_SIZE_MM 300.0 // one maze block = 300mm

// How many mm per AS5600 raw unit
// AS5600 gives 0-4095 per full rotation
#define MM_PER_TICK (WHEEL_CIRCUMFERENCE / 4096.0)

// ─── TCA9548A CONFIG ─────────────────────────────────────────
// Multiplexer I2C address (default 0x70)
#define TCA_ADDRESS 0x70
#define LEFT_CHANNEL 0  // AS5600 left  wheel on channel 0
#define RIGHT_CHANNEL 1 // AS5600 right wheel on channel 1

// ─── OBJECTS ─────────────────────────────────────────────────
TCA9548 tca(TCA_ADDRESS);
AS5600 encoderLeft;
AS5600 encoderRight;

// ─── TRACKING VARIABLES ──────────────────────────────────────
uint16_t leftPrev = 0;  // previous raw angle left
uint16_t rightPrev = 0; // previous raw angle right
float leftDistMM = 0;   // total distance left wheel mm
float rightDistMM = 0;  // total distance right wheel mm
float totalDistMM = 0;  // average of both wheels

// ─────────────────────────────────────────────────────────────
void encoderInit()
{
    Wire.begin(21, 22); // SDA=21, SCL=22

    tca.begin();

    // Init left encoder via channel 0
    tca.selectChannel(LEFT_CHANNEL);
    encoderLeft.begin(4); // 4 = I2C address offset
    if (!encoderLeft.isConnected())
    {
        Serial.println("Left AS5600 not found!");
    }
    else
    {
        Serial.println("Left AS5600 ready!");
    }

    // Init right encoder via channel 1
    tca.selectChannel(RIGHT_CHANNEL);
    encoderRight.begin(4);
    if (!encoderRight.isConnected())
    {
        Serial.println("Right AS5600 not found!");
    }
    else
    {
        Serial.println("Right AS5600 ready!");
    }

    // Store initial positions
    tca.selectChannel(LEFT_CHANNEL);
    leftPrev = encoderLeft.rawAngle();

    tca.selectChannel(RIGHT_CHANNEL);
    rightPrev = encoderRight.rawAngle();

    // Reset distances
    leftDistMM = 0;
    rightDistMM = 0;
    totalDistMM = 0;

    Serial.println("Encoders initialized!");
}

// ─── READ DELTA ──────────────────────────────────────────────
// Returns how many ticks moved since last call
// Handles 0→4095 wraparound correctly
int16_t getDelta(uint16_t current, uint16_t previous)
{
    int16_t delta = (int16_t)current - (int16_t)previous;

    // Handle wraparound
    if (delta > 2048)
        delta -= 4096;
    if (delta < -2048)
        delta += 4096;

    return delta;
}

// ─── UPDATE DISTANCES ────────────────────────────────────────
// Call this every loop iteration
void updateEncoders()
{
    // Read left encoder
    tca.selectChannel(LEFT_CHANNEL);
    uint16_t leftCurrent = encoderLeft.rawAngle();
    int16_t leftDelta = getDelta(leftCurrent, leftPrev);
    leftPrev = leftCurrent;
    leftDistMM += leftDelta * MM_PER_TICK;

    // Read right encoder
    tca.selectChannel(RIGHT_CHANNEL);
    uint16_t rightCurrent = encoderRight.rawAngle();
    int16_t rightDelta = getDelta(rightCurrent, rightPrev);
    rightPrev = rightCurrent;
    rightDistMM += rightDelta * MM_PER_TICK;

    // Average distance = robot's forward movement
    totalDistMM = (leftDistMM + rightDistMM) / 2.0;
}

// ─── RESET DISTANCES ─────────────────────────────────────────
// Call at start of each cell or turn
void resetEncoders()
{
    leftDistMM = 0;
    rightDistMM = 0;
    totalDistMM = 0;

    // Re-read current positions as new baseline
    tca.selectChannel(LEFT_CHANNEL);
    leftPrev = encoderLeft.rawAngle();

    tca.selectChannel(RIGHT_CHANNEL);
    rightPrev = encoderRight.rawAngle();
}

// ─── DISTANCE HELPERS ────────────────────────────────────────
float getLeftDistMM() { return leftDistMM; }
float getRightDistMM() { return rightDistMM; }
float getTotalDistMM() { return totalDistMM; }

// ─── CELL DETECTION ──────────────────────────────────────────
// Returns true when robot has traveled one full maze cell
bool traveledOneCell()
{
    return (totalDistMM >= CELL_SIZE_MM);
}

// Returns how many full cells robot has traveled
int cellstraveled()
{
    return (int)(totalDistMM / CELL_SIZE_MM);
}

// ─── ENCODER DEBUG ───────────────────────────────────────────
void printEncoderData()
{
    Serial.print("L: ");
    Serial.print(leftDistMM);
    Serial.print("mm | R: ");
    Serial.print(rightDistMM);
    Serial.print("mm | Total: ");
    Serial.print(totalDistMM);
    Serial.print("mm | Cells: ");
    Serial.println(cellsovered());
}