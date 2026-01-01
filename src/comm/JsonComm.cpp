#include "JsonComm.h"

JsonComm::JsonComm(Stream &serialPort, size_t docSize)
    : serial(serialPort), bufferSize(docSize) {}

void JsonComm::begin() {
    // Rien à initialiser de spécial
}

bool JsonComm::receiveCommand(JsonDocument &doc) {
    static String inputBuffer = "";

    while (serial.available()) {
        char c = serial.read();
        if (c == '\n' || c == '\r') continue;
        inputBuffer += c;
    }

    if (inputBuffer.length() > 0) {
        DeserializationError error = deserializeJson(doc, inputBuffer);
        if (!error) {
            inputBuffer = "";
            return true;
        }
    }

    return false;
}

void JsonComm::sendResponse(const JsonDocument &doc) {
    serializeJson(doc, serial);
    serial.println();
}

void JsonComm::sendStatus(const char* status, const char* message) {
    DynamicJsonDocument doc(128);
    doc["status"] = status;
    doc["message"] = message;
    sendResponse(doc);
}
