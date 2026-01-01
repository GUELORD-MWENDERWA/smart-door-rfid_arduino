#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

class RelayController {
public:
    RelayController(uint8_t pin, uint16_t openDurationMs = 5000);

    void begin();
    void open();        // active relais, démarre temporisation
    void update();      // à appeler dans loop()
    bool isOpen() const;

private:
    uint8_t relayPin;
    uint16_t duration;
    unsigned long openTimestamp;
    bool state;
};

#endif
