#include "JsonComm.h"

JsonComm::JsonComm(Stream &serialPort, size_t docSize)
    : serial(serialPort),
      bufferSize(docSize)
{}

void JsonComm::begin() {
    Serial.println(F("[JSON] JsonComm initialized"));
}

/* ===== RECEIVE JSON COMMAND ===== */
bool JsonComm::receiveCommand(JsonDocument &doc) {
    static String inputBuffer;

    while (serial.available()) {
        char c = serial.read();
        if (c == '\n' || c == '\r') continue;
        inputBuffer += c;

        if (inputBuffer.length() > bufferSize) {
            Serial.println(F("[JSON][ERROR] Buffer overflow, reset"));
            inputBuffer = "";
            return false;
        }
    }

    if (inputBuffer.length() == 0) return false;

    Serial.print(F("[JSON][RX] "));
    Serial.println(inputBuffer);

    DeserializationError err = deserializeJson(doc, inputBuffer);
    if (err) {
        Serial.print(F("[JSON][ERROR] Parse failed: "));
        Serial.println(err.c_str());
        inputBuffer = "";
        return false;
    }

    Serial.println(F("[JSON][OK] Valid JSON received"));
    inputBuffer = "";
    return true;
}

/* ===== SEND JSON RESPONSE ===== */
void JsonComm::sendResponse(const JsonDocument &doc) {
    Serial.print(F("[JSON][TX] "));
    serializeJson(doc, Serial);
    Serial.println();
}

/* ===== SEND SIMPLE STATUS ===== */
void JsonComm::sendStatus(const char* status, const char* message) {
    DynamicJsonDocument doc(128); doc;
    doc["status"] = status;
    doc["message"] = message;

    Serial.print(F("[JSON][STATUS] "));
    serializeJson(doc, Serial);
    Serial.println();
}

/* ===== SEND CUSTOM DATA ===== */
void JsonComm::sendData(const char* key, const char* value) {
    DynamicJsonDocument doc(128); doc;
    doc[key] = value;
    Serial.print(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    Serial.println();
}

void JsonComm::sendData(const char* key, int value) {
    DynamicJsonDocument doc(128); doc;
    doc[key] = value;
    Serial.print(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    Serial.println();
}

void JsonComm::sendData(const char* key, bool value) {
    DynamicJsonDocument doc(128); doc;
    doc[key] = value;
    Serial.print(F("[JSON][DATA] "));
    serializeJson(doc, Serial);
    Serial.println();
}
