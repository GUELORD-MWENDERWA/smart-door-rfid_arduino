#include "JsonComm.h"
#include "config.h"


JsonComm::JsonComm(Stream &serialPort, size_t docSize)
    : serial(serialPort),
      bufferSize(docSize)
{}

void JsonComm::begin() {
    DEBUG_PRINTLN(F("[JSON] JsonComm initialized"));
}

/* ===== RECEIVE JSON COMMAND ===== */
bool JsonComm::receiveCommand(JsonDocument &doc) {
    static String inputBuffer;

#if DEBUG_SERIAL
    // =================================================
    // MODE DEBUG : tolérant, avec timeout
    // =================================================
    static unsigned long lastRxTime = 0;
    const unsigned long RX_TIMEOUT_MS = 100;

    while (serial.available()) {
        char c = serial.read();
        lastRxTime = millis();

        if (c == '\n' || c == '\r') continue;
        inputBuffer += c;

        if (inputBuffer.length() > bufferSize) {
            DEBUG_PRINTLN(F("[JSON][ERROR] Buffer overflow, reset"));
            inputBuffer = "";
            return false;
        }
    }

    if (inputBuffer.length() == 0) return false;

    // attendre que le JSON semble complet
    if (!inputBuffer.endsWith("}")) {
        if (millis() - lastRxTime > RX_TIMEOUT_MS) {
            DEBUG_PRINTLN(F("[JSON][ERROR] RX timeout, drop buffer"));
            inputBuffer = "";
        }
        return false;
    }

    DEBUG_PRINT(F("[JSON][RX] "));
    DEBUG_PRINTLN(inputBuffer);

#else
    // =================================================
    // MODE PROD : robuste, basé sur délimiteur '\n'
    // =================================================
    while (serial.available()) {
        char c = serial.read();

        if (c == '\r') continue;

        if (c == '\n') {
            // fin de message
            break;
        }

        inputBuffer += c;

        if (inputBuffer.length() > bufferSize) {
            inputBuffer = "";
            return false;
        }
    }

    if (inputBuffer.length() == 0) return false;
#endif

    // =================================================
    // Désérialisation JSON (COMMUNE)
    // =================================================
    DeserializationError err = deserializeJson(doc, inputBuffer);

    if (err) {
#if DEBUG_SERIAL
        DEBUG_PRINT(F("[JSON][ERROR] Parse failed: "));
        DEBUG_PRINTLN(err.c_str());
#endif
        inputBuffer = "";
        return false;
    }

#if DEBUG_SERIAL
    DEBUG_PRINTLN(F("[JSON][OK] Valid JSON received"));
#endif

    inputBuffer = "";
    return true;
}


/* ===== SEND JSON RESPONSE ===== */
void JsonComm::sendResponse(const JsonDocument &doc) {
    DEBUG_PRINT(F("[JSON][TX] "));
    serializeJson(doc, Serial);
    DEBUG_PRINTLN();
}

/* ===== SEND SIMPLE STATUS ===== */
void JsonComm::sendStatus(const char* status, const char* message) {
    StaticJsonDocument<128> doc; doc;
    doc["status"] = status;
    doc["message"] = message;

    DEBUG_PRINT(F("[JSON][STATUS] "));
    serializeJson(doc, Serial);
    DEBUG_PRINTLN();
}

/* ===== SEND CUSTOM DATA ===== */
void JsonComm::sendData(const char* key, const char* value) {
    StaticJsonDocument<128> doc; doc;
    doc[key] = value;
    DEBUG_PRINT(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    DEBUG_PRINTLN();
}

void JsonComm::sendData(const char* key, int value) {
    StaticJsonDocument<128> doc; doc;
    doc[key] = value;
    DEBUG_PRINT(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    DEBUG_PRINTLN();
}

void JsonComm::sendData(const char* key, bool value) {
    StaticJsonDocument<128> doc; doc;
    doc[key] = value;
    DEBUG_PRINT(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    DEBUG_PRINTLN();
}
