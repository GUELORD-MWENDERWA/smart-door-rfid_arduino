#ifndef JSON_COMM_H
#define JSON_COMM_H

#include <Arduino.h>
#include <ArduinoJson.h>

/*
  JsonComm
  - Framing: newline-terminated JSON messages '\n'
  - Buffer limit: MAX_LINE (256)
  - Accepts possibly multiple messages in one serial burst
  - Parses JSON into a StaticJsonDocument provided by the caller
  - If incoming JSON has no "id", a generated id "evt-<n>" is inserted into the returned document
  - Always appends '\n' after outgoing messages
  - Provides helpers to send ack / error / system responses
*/

class JsonComm {
public:
    // Construct with a Stream reference (Serial)
    JsonComm(Stream &serialPort);

    // No-op for now, kept for API symmetry
    void begin(unsigned long baud = 115200);

    // non-blocking call: returns true if a full JSON message has been assembled and deserialized
    // outDoc must be a StaticJsonDocument with sufficient capacity (recommended 256)
    // On success: outDoc contains parsed JSON and guaranteed to contain "id" (either provided by sender or added here)
    // Caller should inspect fields (cmd, params, etc.)
    bool receiveCommand(StaticJsonDocument<256> &outDoc);

    // Generic sendResponse: accept any JsonDocument/StaticJsonDocument size via template
    template <typename T>
    bool sendResponse(const T &doc) {
        // serialize directly to the stream and append newline
        serializeJson(doc, serial);
        serial.print('\n');
        return true;
    }

    // Convenience: send ack with optional error message
    bool sendAck(const char *id, const char *result = "ok", const char *error = nullptr);

    // Convenience: send error with optional id (if id==nullptr it will still send an error without id)
    bool sendError(const char *id, const char *errorMsg);

    // Helper to generate an event id (evt-<counter>)
    void generateLocalEventId(char *buf, size_t bufLen);

private:
    Stream &serial;
    static const size_t MAX_LINE = 256; // includes terminating '\0'
    char buffer[MAX_LINE];
    size_t bufLen;
    unsigned long lastReadMs;
    uint32_t localCounter;

    // Process a single complete line (null-terminated). Returns true if processed and outDoc filled.
    bool processLine(const char *line, StaticJsonDocument<256> &outDoc);

    // Internal: shift buffer after consuming n bytes
    void consumeBytes(size_t n);

    // Internal: safely append generated id into outDoc (mutating it) if missing.
    void ensureId(StaticJsonDocument<256> &doc);

    // Internal: send raw C string + newline
    bool sendRaw(const char *txt);
};

#endif // JSON_COMM_H