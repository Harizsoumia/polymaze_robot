# 🤖 POLYMAZE Robot — Team README

> Autonomous maze-solving robot built for **POLYMAZE 2026** competition  
> Organized by **Vision & Innovation Club (VIC)**  
> Theme: 2026 Football World Cup 🏆

---

// red led pin , ir sensors

## 📁 Project Structure

```
src/
├── main.cpp          → Main loop + state machine
├── motor.h           → Motor control (TB6612FNG)
├── sensor.h          → Line + wall sensors (QTR-8 + IR)
├── pid.h             → PID line following algorithm
├── intersection.h    → Intersection detection + turn logic
├── color.h           → RGB color detection (checkpoints)
├── buzzer.h          → End of maze whistle minigame
└── maze.h            → Maze solving (Left/Right Hand + Memory + Speed Run)
```

---

## ⚙️ Hardware Components

| Component       | Model               | Purpose                       |
| --------------- | ------------------- | ----------------------------- |
| Microcontroller | ESP32               | Brain of the robot            |
| Motor Driver    | TB6612FNG           | Controls 2 DC motors          |
| Line Sensor     | QTR-8RC (8 sensors) | Follows black line            |
| Wall Sensors    | IR x2 (left, right) | Detects maze walls            |
| Color Sensor    | TCS34725            | Detects red/green checkpoints |
| Encoders        | AS5600 x2           | Wheel speed feedback          |
| I2C Multiplexer | TCA9548A            | Handles both AS5600 encoders  |
| Buzzer          | Piezo               | Final whistle minigame        |
| LEDs            | Red + Green         | Checkpoint visual response    |
| Motors          | DC Gear Motor 3/6V  | Drive wheels                  |
| Battery         | 18650 x2 (parallel) | Power supply                  |
| Switch          | Interrupteur        | Explore ↔ Speed Run mode      |

---

## 🔌 Pin Assignment

### TB6612FNG (Motor Driver)

```
TB6612 Pin    →    ESP32 Pin    Function
─────────────────────────────────────────
VCC           →    3.3V         Logic power
VM            →    Battery+     Motor power (3-6V)
GND           →    GND          Common ground
STBY          →    GPIO 15      Enable driver (must be HIGH)
AIN1          →    GPIO 19      Motor A direction
AIN2          →    GPIO 18      Motor A direction
PWMA          →    GPIO 5       Motor A speed (PWM)
BIN1          →    GPIO 17      Motor B direction
BIN2          →    GPIO 16      Motor B direction
PWMB          →    GPIO 4       Motor B speed (PWM)
AO1/AO2       →    Motor A      Left wheel
BO1/BO2       →    Motor B      Right wheel
```

### QTR-8RC (Line Sensor)

```
QTR Pin    →    ESP32 Pin    Sensor Position
────────────────────────────────────────────
VCC        →    3.3V
GND        →    GND
LEDON      →    GPIO 2       IR emitter control
S1         →    GPIO 36      Far LEFT
S2         →    GPIO 39
S3         →    GPIO 34
S4         →    GPIO 35      Center left
S5         →    GPIO 32      Center right
S6         →    GPIO 33
S7         →    GPIO 27
S8         →    GPIO 14      Far RIGHT
```

### Other Components

```
Component      ESP32 Pin    Notes
──────────────────────────────────────────────
IR Left        GPIO 26      Wall detection
IR Right       GPIO 25      Wall detection
TCS34725 SDA   GPIO 21      I2C (shared bus)
TCS34725 SCL   GPIO 22      I2C (shared bus)
TCA9548A SDA   GPIO 21      I2C multiplexer
TCA9548A SCL   GPIO 22      I2C multiplexer
AS5600 #1      via TCA9548A Left encoder
AS5600 #2      via TCA9548A Right encoder
Buzzer         GPIO 13      PWM tone output
Red LED        GPIO 23      Through 100Ω resistor
Green LED      GPIO 12      Through 100Ω resistor
Switch         GPIO 0       Explore/Speed Run toggle
```

### ⚠️ Critical Wiring Rules

```
1. ALL GND pins must share common ground
   (ESP32 + TB6612 + QTR + IR + Battery -)

2. Never put 5V into ESP32 GPIO → max 3.3V

3. Power all sensors from ESP32 3.3V pin
   → their output will also be 3.3V → safe

4. STBY pin on TB6612 must be HIGH → motors work
   If LOW → motors disabled regardless of code

5. 100Ω resistor in series with each LED → always
```

---

## 📦 Libraries Required

Add to `platformio.ini`:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps =
    pololu/QTRSensors @ ^4.0.0
    adafruit/Adafruit TCS34725 @ ^1.4.2
    adafruit/Adafruit BusIO @ ^1.14.0
```

---

## 🧠 How The Code Works

### 1. motor.h — Motor Control

Controls the TB6612FNG driver using ESP32's LEDC PWM system.

```
Key functions:
motorsInit()        → setup pins + PWM channels
setMotorA(speed)    → speed: -255 (back) to +255 (forward)
setMotorB(speed)    → same for right motor
driveForward(speed) → both motors forward
turnLeft(speed)     → left back, right forward (tank turn)
turnRight(speed)    → left forward, right back
stopMotors()        → both motors stop
```

**Why LEDC instead of analogWrite?**  
ESP32 doesn't support `analogWrite()`. It uses the LEDC peripheral (LED Controller) for PWM signals. Same concept, different function name.

---

### 2. sensor.h — Sensor Reading

Reads QTR-8 line sensors and IR wall sensors.

```
Key functions:
sensorInit()        → setup all sensor pins
calibrateSensors()  → run at startup, sweep over line
readLine()          → returns position 0-7000
                      0    = far left
                      3500 = center
                      7000 = far right
isLineLost()        → true if no sensor sees line
isIntersection()    → true if S1 + S8 both see line
isDeadEnd()         → true if only center sensors active
wallLeft()          → true if left IR detects wall
wallRight()         → true if right IR detects wall
```

**Position value explained:**

```
Sensor:   S1    S2    S3    S4    S5    S6    S7    S8
Position: 0    1000  2000  3000  4000  5000  6000  7000
                           ↑
                        CENTER (3500)
```

---

### 3. pid.h — PID Line Following

Calculates motor corrections to keep robot centered on line.

```
error      = position - 3500
correction = (Kp × error) + (Ki × integral) + (Kd × derivative)
leftSpeed  = BASE_SPEED - correction
rightSpeed = BASE_SPEED + correction
```

**Tuning guide:**

```
Problem                    Fix
──────────────────────────────────────────────
Robot zigzags badly    →   reduce Kp (try 0.04)
Robot drifts slowly    →   increase Kp (try 0.12)
Robot overshoots turns →   increase Kd (try 0.5)
Robot stutters         →   decrease Kd (try 0.15)
Robot curves straight  →   add small Ki (0.001)
```

**Starting values:**

```cpp
float Kp = 0.08;
float Ki = 0.0;
float Kd = 0.3;
```

---

### 4. intersection.h — Intersection Handling

Detects intersection type using QTR + IR sensors and executes turns.

```
Intersection types:
TURN_LEFT   → only left path available
TURN_RIGHT  → only right path available
T_LEFT      → left + straight available
T_RIGHT     → right + straight available
CROSS       → all directions available
DEAD_END    → no path forward

Turn functions:
doTurnLeft()        → forward 100ms → turn left TURN_DURATION ms
doTurnRight()       → forward 100ms → turn right TURN_DURATION ms
doUTurn()           → turn right 2×TURN_DURATION ms (180°)
goStraightThrough() → push forward 200ms through intersection
```

**Tuning values to adjust after testing:**

```cpp
#define TURN_SPEED      120   // turn motor speed
#define TURN_DURATION   300   // ms — adjust in 50ms steps
```

---

### 5. color.h — Checkpoint Detection

Uses TCS34725 RGB sensor to detect colored bands on maze floor.

```
detectColor() returns:
COLOR_RED    → red band detected   → light red LED
COLOR_GREEN  → green band detected → light green LED
COLOR_BLACK  → on black line
COLOR_WHITE  → on white surface
COLOR_NONE   → unclear reading
```

**Why normalization?**

```
Raw values change with lighting conditions.
Dividing by clear channel (c) normalizes them:
rNorm = (r / c) × 255

Same color reads same value regardless of brightness ✅
Critical for POLYMAZE — lighting varies per venue!
```

**Calibration — do before every competition:**

```
1. Call calibrateColor() in setup() temporarily
2. Place sensor over red band → note R,G,B values
3. Place sensor over green band → note R,G,B values
4. Update thresholds in color.h
5. Remove calibrateColor() from setup()
```

---

### 6. buzzer.h — End of Maze Whistle

Plays 3 football whistles when robot reaches finish line.

```
finalWhistle():
→ beep 300ms
→ pause 200ms
→ beep 300ms
→ pause 200ms
→ beep 600ms (longer = final whistle feel)

isFinishLine():
→ returns true if 6+ sensors see black
→ finish line is 20×20cm black square (POLYMAZE spec)
→ much wider than normal 1.5cm line → easy to distinguish
```

**Worth 4 points in POLYMAZE scoring!**

---

### 7. maze.h — Maze Solver

**Strategy: Left/Right Hand Rule + Memory + Speed Run**

```
EXPLORE MODE (switch UP):
├── Robot uses Left OR Right Hand Rule
├── Left Hand:  prefer left → straight → right → U-turn
├── Right Hand: prefer right → straight → left → U-turn
├── Every decision saved to pathMemory[]
└── When finish reached → whistles → stops

SPEED RUN MODE (switch DOWN):
├── Robot replays pathMemory[] exactly
├── No decision making — just executes stored turns
├── Full sprint speed (SPRINT_SPEED)
└── Fastest possible run ✅
```

**Competition day workflow:**

```
1. Look at maze entrance (10 seconds)
2. Flip algo switch:
   └── More left paths → LEFT HAND
   └── More right paths → RIGHT HAND
3. Flip mode switch → EXPLORE (UP)
4. Place robot at start → release
5. Robot explores → reaches finish → whistles
6. Pick up robot
7. Flip mode switch → SPEED RUN (DOWN)
8. Place robot at start → release
9. Robot sprints to finish at full speed 🚀
```

---

### 8. main.cpp — State Machine

Robot is always in exactly one state:

```
         ┌─────────────────┐
    ┌───→│  FOLLOWING_LINE │←──────┐
    │    └────────┬────────┘       │
    │             │                │
    │        intersection?    line found
    │             │                │
    │    ┌────────▼────────┐       │
    │    │ AT_INTERSECTION │       │
    │    └────────┬────────┘       │
    │        turn done             │
    └─────────────┘                │
                                   │
         line lost?                │
              │                    │
    ┌─────────▼────────┐           │
    │    LINE_LOST     ├───────────┘
    └──────────────────┘

    finish line?
         │
    ┌────▼─────┐
    │ FINISHED │ → stop → whistle
    └──────────┘
```

---

## 🧪 Testing Sequence

**Always test in this order — never skip steps:**

### Test 1 — Motors only

```cpp
// main.cpp content:
#include "motor.h"
void setup() { Serial.begin(115200); motorsInit(); }
void loop() {
    setMotorA(150);  delay(1000);
    setMotorA(-150); delay(1000);
    setMotorB(150);  delay(1000);
    setMotorB(-150); delay(1000);
    stopMotors();    delay(2000);
}
```

Expected: each motor spins forward then backward.  
If wrong direction → swap AIN1 ↔ AIN2 in motor.h

### Test 2 — Sensors only

```cpp
#include "sensor.h"
void setup() { Serial.begin(115200); sensorInit(); calibrateSensors(); }
void loop() {
    readLine();
    for (int i = 0; i < NUM_SENSORS; i++) {
        Serial.print(sensorValues[i]); Serial.print("\t");
    }
    Serial.print("| Pos: "); Serial.println(position);
    delay(100);
}
```

Expected: values 0-100 on white, 800-1000 on black line.

### Test 3 — IR wall sensors

```cpp
#include "sensor.h"
void setup() { Serial.begin(115200); sensorInit(); }
void loop() {
    Serial.print("L:"); Serial.print(wallLeft());
    Serial.print(" R:"); Serial.println(wallRight());
    delay(200);
}
```

Expected: 0 = no wall, 1 = wall detected.

### Test 4 — PID line following

Full `main.cpp` with only `pidFollow()` in loop.  
Expected: robot follows straight line smoothly.

### Test 5 — Full system

Complete `main.cpp` with all modules.  
Expected: robot navigates maze, detects checkpoints, whistles at finish.

---

## 🏆 POLYMAZE Scoring

```
Action                          Points
────────────────────────────────────────────────────
Complete maze                → (300 - time_seconds)
Red checkpoint detected      → +2 points
Green checkpoint detected    → +2 points
Final whistle (3 beeps)      → +4 points
Robot aesthetics             → +1 point (homologation)
Cable management / PCB       → +2 points (homologation)

Wall hit penalty             → -3 points each
Hand touch penalty           → -7 points each (max 3)
────────────────────────────────────────────────────
Formula (maze completed):
Total = (300 - time) + bonus - penalties

Formula (maze not completed):
Total = (blocks covered) + bonus - penalties
```

---

## 🐛 Common Problems & Fixes

```
Problem                          Fix
────────────────────────────────────────────────────────────
Motors don't spin             → Check STBY pin HIGH, check GND
Motor spins wrong direction   → Swap AIN1 ↔ AIN2 (or BIN1 ↔ BIN2)
All sensor values = 0         → Check EMITTER_PIN wiring
All sensor values = 1000      → Recalibrate sensors
Robot zigzags on line         → Reduce Kp or increase Kd
Robot doesn't turn enough     → Increase TURN_DURATION
Robot overshoots turns        → Decrease TURN_SPEED
Color sensor not found        → Check I2C wiring (SDA/SCL)
Upload fails                  → Check COM port in platformio.ini
Serial Monitor garbage        → Set baud to 115200
```

---

## 👥 Team

- **Competition:** POLYMAZE 2026
- **Organizer:** Vision & Innovation Club (VIC)
- **University:** USTHB, Algeria
- **Framework:** Arduino on ESP32 (PlatformIO)

---

## 📅 Development Roadmap

- [x] Motor control layer
- [x] QTR-8 sensor reading + calibration
- [x] PID line following
- [x] Intersection detection + handling
- [x] RGB color checkpoint detection
- [x] Buzzer end-of-maze whistle
- [x] Left/Right Hand maze solver + memory
- [x] Speed run mode
- [ ] AS5600 encoder feedback (Phase 7)
- [ ] Flood Fill algorithm (if time allows)
- [ ] Full maze testing + PID tuning
- [ ] Competition day calibration
