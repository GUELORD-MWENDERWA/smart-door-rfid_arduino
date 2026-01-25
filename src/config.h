#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Debug macros — set to no-op to save flash on UNO (define DEBUG_ENABLE to enable)
#ifdef DEBUG_ENABLE
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT(x) Serial.print(x)
#else
  // no-op to reduce flash usage
  #define DEBUG_PRINTLN(x) ((void)0)
  #define DEBUG_PRINT(x)  ((void)0)
#endif

// EEPROM / system defaults
#define EEPROM_MAGIC 0xA5A5
#define EEPROM_VERSION 1

// Admin PIN defaults
#define DEFAULT_ADMIN_PIN "123"

// Relay default open time (ms)
#define RELAY_DEFAULT_OPEN_TIME 5000

#endif // CONFIG_H