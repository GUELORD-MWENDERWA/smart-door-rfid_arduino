#include "JsonComm.h"

#ifndef DEBUG_PRINTLN
// Keep existing DEBUG_PRINT macros compatibility if defined elsewhere.
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#endif

JsonComm::JsonComm(Stream &serialPort)
    : serial(serialPort),
      bufLen(0),
      lastReadMs(0),
      localCounter(0)
{
    // initialize buffer
    memset(buffer, 0, sizeof(buffer));
}

void JsonComm::begin(unsigned long baud) {
    // Serial begin is handled by main (keep no-op to avoid double begin).
    (void)baud;
}

bool JsonComm::receiveCommand(StaticJsonDocument<256> &outDoc) {
    // Read available bytes and append to internal buffer
    while (serial.available() > 0) {
        int c = serial.read();
        if (c < 0) break;

        // Accept CR but treat only LF as terminator
        if (c == '\r') continue;

        if (bufLen + 1 >= MAX_LINE) {
            // Buffer overflow -> drop buffer and send error
            DEBUG_PRINTLN(F("[JSONCOMM] RX buffer overflow, dropping"));
            // Send error (no id, best effort)
            sendError(nullptr, "message_too_long");
            bufLen = 0; // drop
            memset(buffer, 0, sizeof(buffer));
            // continue reading (drain)
            continue;
        }

        buffer[bufLen++] = (char)c;

        // If newline detected, process one line (could have multiple lines later)
        if (c == '\n') {
            // Null-terminate the line (replace newline with '\0' to ease parsing)
            buffer[bufLen - 1] = '\0';

            // process first line: since buffer might contain multiple messages if serial delivered them in burst,
            // we locate the first '\0'-terminated line and process it, then shift remaining bytes.
            bool ok = false;
            // Prepare a temporary StaticJsonDocument for parsing to avoid corrupting caller doc on fail
            StaticJsonDocument<256> tmpDoc;
            ok = processLine(buffer, tmpDoc);

            // If success, move tmpDoc content into outDoc by serializing+deserializing
            if (ok) {
                // Copy parsed JSON to caller doc
                // Serialize tmpDoc into a small buffer then deserialize into outDoc
                char tmpBuf[MAX_LINE];
                size_t n = serializeJson(tmpDoc, tmpBuf, sizeof(tmpBuf));
                DeserializationError err = deserializeJson(outDoc, tmpBuf, n);
                if (err) {
                    DEBUG_PRINTLN(F("[JSONCOMM] Unexpected deserialize error copying doc"));
                    // return false (malformed copy) but continue with buffer consumption
                } else {
                    // Ensure id exists (if not present, ensureId will generate one)
                    ensureId(outDoc);
                    // consume this line from buffer (up to and including null we set)
                    size_t consumed = strlen(buffer) + 1; // +1 because we replaced '\n' with '\0'
                    consumeBytes(consumed);
                    return true;
                }
            } else {
                // If processing failed (invalid json or error), consume this line and continue
                size_t consumed = strlen(buffer) + 1;
                consumeBytes(consumed);
                // continue loop to attempt next messages in buffer
            }
        }
    }

    // No full message ready
    return false;
}

bool JsonComm::processLine(const char *line, StaticJsonDocument<256> &outDoc) {
    if (!line || line[0] == '\0') {
        // empty line, ignore
        return false;
    }

    DEBUG_PRINT(F("[JSONCOMM] RX LINE: "));
    DEBUG_PRINTLN(line);

    // Try deserialize
    DeserializationError err = deserializeJson(outDoc, line);
    if (err) {
        DEBUG_PRINT(F("[JSONCOMM] JSON parse error: "));
        DEBUG_PRINTLN(err.c_str());

        // Send a standard error back (no id whenever we cannot reliably parse it)
        sendError(nullptr, "invalid_json");

        return false;
    }

    // If parse OK, ensure it's an object
    if (!outDoc.is<JsonObject>()) {
        sendError(nullptr, "json_not_object");
        return false;
    }

    // This implementation will not reject messages missing "cmd" here;
    // higher-level logic (main) can decide. We WILL ensure an "id" is present (see ensureId).
    ensureId(outDoc);

    return true;
}

void JsonComm::ensureId(StaticJsonDocument<256> &doc) {
    // Note: containsKey is deprecated in newer ArduinoJson versions but still available.
    // We'll check for presence by testing doc["id"].is<const char*>() where appropriate later.
    if (!doc.containsKey("id")) {
        // generate id evt-<counter>
        char idbuf[24];
        generateLocalEventId(idbuf, sizeof(idbuf));
        doc["id"] = idbuf;
        DEBUG_PRINT(F("[JSONCOMM] Added generated id: "));
        DEBUG_PRINTLN(idbuf);
    }
}

void JsonComm::consumeBytes(size_t n) {
    if (n == 0) return;
    // Find the length of current buffer (bufLen)
    if (n >= bufLen) {
        bufLen = 0;
        buffer[0] = '\0';
        return;
    }
    // Move remaining bytes to front
    memmove(buffer, buffer + n, bufLen - n);
    size_t newLen = bufLen - n;
    // Make sure we still have zero termination at the end (we keep raw bytes, not necessarily null-terminated)
    if (newLen < MAX_LINE) buffer[newLen] = '\0';
    bufLen = newLen;
}

bool JsonComm::sendRaw(const char *txt) {
    if (!txt) return false;
    serial.print(txt);
    serial.print('\n');
    return true;
}

bool JsonComm::sendAck(const char *id, const char *result, const char *error) {
    StaticJsonDocument<128> doc;
    if (id && id[0] != '\0') doc["id"] = id;
    doc["type"] = "ack";
    doc["result"] = result ? result : "ok";
    if (error) doc["error"] = error;
    serializeJson(doc, serial);
    serial.print('\n');
    return true;
}

bool JsonComm::sendError(const char *id, const char *errorMsg) {
    StaticJsonDocument<128> doc;
    if (id && id[0] != '\0') doc["id"] = id;
    doc["type"] = "error";
    doc["error"] = errorMsg ? errorMsg : "unknown_error";
    serializeJson(doc, serial);
    serial.print('\n');
    return true;
}

void JsonComm::generateLocalEventId(char *buf, size_t bufLen) {
    if (!buf || bufLen == 0) return;
    // Simple counter based id
    unsigned long c = ++localCounter;
    // Format "evt-<counter>"
    snprintf(buf, bufLen, "evt-%lu", c);
}