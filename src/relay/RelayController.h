#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

/*
  RelayController
  - Non-blocking relay control with open duration handled via millis()
  - isOpen() returns current relay logical state
*/

class RelayController {
public:
    RelayController(uint8_t pin, unsigned long openTimeMs = 5000);

    void begin();
    void update();

    void open();   // open relay (start timer)
    void close();  // force close
    bool isOpen() const;

private:
    uint8_t pin;
    unsigned long openTime;
    unsigned long openSince;
    bool state; // true=open
};

#endif // RELAY_CONTROLLER_H