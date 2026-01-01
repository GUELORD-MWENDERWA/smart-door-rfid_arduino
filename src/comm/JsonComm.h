#ifndef JSON_COMM_H
#define JSON_COMM_H

#include <Arduino.h>
#include <ArduinoJson.h>

class JsonComm {
public:
    JsonComm(Stream &serialPort, size_t docSize = 256);

    void begin();
    bool receiveCommand(JsonDocument &doc); // lit Serial, renvoie true si JSON valide reçu
    void sendResponse(const JsonDocument &doc); // envoie JSON via Serial
    void sendStatus(const char* status, const char* message);

private:
    Stream &serial;
    size_t bufferSize;
};

#endif
