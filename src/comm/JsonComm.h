#ifndef JSON_COMM_H
#define JSON_COMM_H

#include <Arduino.h>
#include <ArduinoJson.h>

class JsonComm {
public:
    // Constructeur : port série + taille max du buffer JSON
    JsonComm(Stream &serialPort, size_t docSize = 256);

    // Initialise la communication JSON
    void begin();

    // Lit le port série et tente de parser un JSON
    bool receiveCommand(JsonDocument &doc);

    // Envoie un document JSON
    void sendResponse(const JsonDocument &doc);

    // Envoie une réponse simple {status, message}
    void sendStatus(const char* status, const char* message);

    // Envoie une paire clé/valeur simple
    void sendData(const char* key, const char* value);
    void sendData(const char* key, int value);
    void sendData(const char* key, bool value);

private:
    Stream &serial;     // Port série utilisé
    size_t bufferSize;  // Taille max du buffer JSON
};

#endif
