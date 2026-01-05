#include "RelayController.h"
#include "config.h"


RelayController::RelayController(uint8_t pin, uint16_t openDurationMs)
    : relayPin(pin),
      duration(openDurationMs),
      openTimestamp(0),
      state(false)
{
}

void RelayController::begin() {
    DEBUG_PRINTLN(F("[RELAY] Initialisation"));

    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    DEBUG_PRINT(F("[RELAY] Pin="));
    DEBUG_PRINT(relayPin);
    DEBUG_PRINT(F(" Duration(ms)="));
    DEBUG_PRINTLN(duration);
}

void RelayController::open() {
    if (state) {
        DEBUG_PRINTLN(F("[RELAY] Open ignored (already open)"));
        return;
    }

    state = true;
    openTimestamp = millis();
    digitalWrite(relayPin, HIGH);

    DEBUG_PRINTLN(F("[RELAY] OPEN -> relay activated"));
}

void RelayController::update() {
    if (!state) return;

    unsigned long now = millis();
    if (now - openTimestamp >= duration) {
        state = false;
        digitalWrite(relayPin, LOW);

        DEBUG_PRINTLN(F("[RELAY] CLOSE -> relay deactivated"));
    }
}

bool RelayController::isOpen() const {
    return state;
}
