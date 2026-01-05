#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ================= DEBUG CONFIG =================
#define DEBUG_SERIAL 0   // 1 = actif, 0 = désactivé

#if DEBUG_SERIAL
  #define DEBUG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...)  Serial.println(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
#endif

#endif // CONFIG_H
