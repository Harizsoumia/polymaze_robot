#include <Arduino.h>
#include "motor.h"

void setup()
{
    Serial.begin(115200);
    motorsInit();
    Serial.println("Test moteurs : A et B avant 2s");
    delay(1000);
}

void loop()
{
    // Faire tourner les deux moteurs en avant
    setMotorA(120);
    setMotorB(120);
    delay(2000);

    stopMotors();
    Serial.println("Arrêt des moteurs");
    delay(2000);

    // Recommence le test à intervalle régulier
}