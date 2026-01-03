#include "RelayController.h"

RelayController::RelayController(uint8_t pin, uint16_t openDurationMs)
    : relayPin(pin),
      duration(openDurationMs),
      openTimestamp(0),
      state(false)
{
}

void RelayController::begin() {
    Serial.println(F("[RELAY] Initialisation"));

    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    Serial.print(F("[RELAY] Pin="));
    Serial.print(relayPin);
    Serial.print(F(" Duration(ms)="));
    Serial.println(duration);
}

void RelayController::open() {
    if (state) {
        Serial.println(F("[RELAY] Open ignored (already open)"));
        return;
    }

    state = true;
    openTimestamp = millis();
    digitalWrite(relayPin, HIGH);

    Serial.println(F("[RELAY] OPEN -> relay activated"));
}

void RelayController::update() {
    if (!state) return;

    unsigned long now = millis();
    if (now - openTimestamp >= duration) {
        state = false;
        digitalWrite(relayPin, LOW);

        Serial.println(F("[RELAY] CLOSE -> relay deactivated"));
    }
}

bool RelayController::isOpen() const {
    return state;
}
