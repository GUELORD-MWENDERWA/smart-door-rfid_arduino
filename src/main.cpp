#include <Arduino.h>

#include "fsm/FSMController.h"
#include "rfid/RFIDModule.h"
#include "eeprom/EEPROMStore.h"
#include "keypad/KeypadModule.h"
#include "ui/UIFeedback.h"
#include "relay/RelayController.h"
#include "comm/JsonComm.h"

/* ===== PINS ===== */
const uint8_t SS_PIN     = 10;
const uint8_t RST_PIN    = 9;
const uint8_t LED_GREEN  = 7;
const uint8_t LED_RED    = 6;
const uint8_t BUZZER     = 2;
const uint8_t RELAY_PIN  = 8;

/* ===== KEYPAD 4x4 ===== */
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS * COLS] = {
    '1','2','3','A',
    '4','5','6','B',
    '7','8','9','C',
    '*','0','#','D'
};

byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2, 3};

/* ===== MODULES ===== */
EEPROMStore     eeprom;
RFIDModule      rfid(SS_PIN, RST_PIN);
RelayController relay(RELAY_PIN, 5000);
UIFeedback      ui(LED_GREEN, LED_RED, BUZZER);
KeypadModule    keypad(keys, rowPins, colPins, ROWS, COLS, "123", BUZZER);
JsonComm        comm(Serial);

/* ===== FSM ===== */
FSMController fsm;

void setup() {
    Serial.begin(9600);
    Serial.println(F("\n=== SYSTEM START ==="));

    eeprom.begin();
    rfid.begin();
    relay.begin();
    ui.begin();
    keypad.begin();
    keypad.changeAdminPIN(eeprom.readAdminPIN()); // <-- synchronisation PIN EEPROM / KeypadModule
    comm.begin();

    Serial.println(F("[SETUP] Init complete"));
}

void loop() {
    keypad.update();
    relay.update();

    /* =====================================================
       SERIAL JSON = SOURCE DE COMMANDE ALTERNATIVE AU KEYPAD
       ===================================================== */
    static bool serialCmdReady = false;
    static String serialCmd = "";

    if (!serialCmdReady) {
        DynamicJsonDocument rxDoc(256);

        if (comm.receiveCommand(rxDoc)) {
            if (rxDoc.containsKey("cmd")) {
                serialCmd = rxDoc["cmd"].as<String>();
                serialCmd.trim();

                if (serialCmd.length() > 0) {
                    serialCmdReady = true;
                    Serial.print(F("[SERIAL CMD READY] "));
                    Serial.println(serialCmd);
                }
            }
        }
    }

    /* =====================================================
       DÉTECTION COMMANDE (KEYPAD OU SERIAL)
       ===================================================== */
    if (
        (keypad.isCommandReady() || serialCmdReady) &&
        fsm.getState() == SystemState::IDLE
    ) {
        fsm.onCommandDetected();
    }

    // Détection badge RFID
    if (rfid.poll() && rfid.hasNewCard()) {
        fsm.onBadgeDetected();
    }

    fsm.update();

    // ==== ACTIONS PRINCIPALES ====
    switch (fsm.getAction()) {

        case FSMAction::VALIDATE_BADGE: {
            uint8_t uid[EEPROMStore::UID_SIZE];
            rfid.getUID(uid);
            bool ok = eeprom.badgeExists(uid);
            fsm.onBadgeValidationResult(ok);

            DynamicJsonDocument doc(128);
            doc["status"] = ok ? "success" : "error";
            doc["type"] = "badge";
            doc["access_granted"] = ok;
            comm.sendResponse(doc);

            fsm.clearAction();
            break;
        }

        case FSMAction::REQUEST_ADMIN_AUTH: {
            String cmd;

            if (serialCmdReady) {
                cmd = serialCmd;
                serialCmdReady = false;
                serialCmd = "";
            } else {
                if (!keypad.isCommandReady()) break;
                cmd = keypad.getCommand();
            }

            keypad.changeAdminPIN(eeprom.readAdminPIN());

            bool ok = keypad.checkAdminPIN(cmd);
            fsm.onAdminAuthResult(ok);

            DynamicJsonDocument doc(128);
            doc["status"] = ok ? "success" : "error";
            doc["type"] = "admin_auth";
            doc["access_granted"] = ok;
            if (!ok) doc["locked"] = keypad.isLocked();
            comm.sendResponse(doc);

            fsm.clearAction();
            break;
        }

        case FSMAction::EXECUTE_COMMAND: {
            String cmd;

            if (serialCmdReady) {
                cmd = serialCmd;
                serialCmdReady = false;
                serialCmd = "";
            } else {
                if (!keypad.isCommandReady()) break;
                cmd = keypad.getCommand();
            }

            cmd.replace("#", "");

            DynamicJsonDocument doc(256);
            doc["type"] = "command";

            if (cmd == "11") {
                ui.signal(FeedbackType::SCAN_BADGE);
                fsm.setState(SystemState::WAIT_ADD_BADGE);
                fsm.clearAction();
                doc["status"] = "scan_required";
                doc["command"] = "add_badge";

            } else if (cmd == "12") {
                ui.signal(FeedbackType::SCAN_BADGE);
                fsm.setState(SystemState::WAIT_REMOVE_BADGE);
                fsm.clearAction();
                doc["status"] = "scan_required";
                doc["command"] = "remove_badge";

            } else if (cmd == "13") {
                doc["status"] = "success";
                doc["total_badges"] = eeprom.getBadgeCount();
                JsonArray badges = doc.createNestedArray("badges");

                for (uint8_t i = 0; i < eeprom.getBadgeCount(); i++) {
                    uint8_t uid[EEPROMStore::UID_SIZE];
                    for (uint8_t j = 0; j < EEPROMStore::UID_SIZE; j++) {
                        uid[j] = EEPROM.read(2 + i * EEPROMStore::UID_SIZE + j);
                    }
                    char uidStr[18];
                    sprintf(uidStr, "%02X %02X %02X %02X %02X",
                            uid[0], uid[1], uid[2], uid[3], uid[4]);
                    badges.add(uidStr);
                }

                fsm.onExecutionDone();

            } else if (cmd == "14") {
                ui.signal(FeedbackType::CONFIRM_RESET);
                fsm.setState(SystemState::WAIT_RESET_CONFIRM);
                fsm.clearAction();
                doc["status"] = "confirm_reset";

            } else if (cmd.startsWith("99")) {
                String newPin = cmd.substring(2);
                if (newPin.length() >= 3 && newPin.length() <= 6) {
                    if (keypad.changeAdminPIN(newPin)) {
                        eeprom.writeAdminPIN(newPin);
                        ui.signal(FeedbackType::ACCESS_GRANTED);
                        doc["status"] = "success";
                        doc["message"] = "Admin PIN updated";
                    } else {
                        ui.signal(FeedbackType::ERROR);
                        doc["status"] = "error";
                        doc["message"] = "Invalid PIN";
                    }
                } else {
                    ui.signal(FeedbackType::ERROR);
                    doc["status"] = "error";
                    doc["message"] = "PIN length must be 3-6 digits";
                }
                fsm.onExecutionDone();

            } else {
                ui.signal(FeedbackType::ERROR);
                doc["status"] = "error";
                doc["message"] = "Unknown command";
                fsm.onExecutionDone();
            }

            comm.sendResponse(doc);
            break;
        }

        case FSMAction::OPEN_DOOR: {
            relay.open();
            ui.signal(FeedbackType::ACCESS_GRANTED);

            DynamicJsonDocument doc(128);
            doc["status"] = "success";
            doc["action"] = "open_door";
            comm.sendResponse(doc);

            fsm.onExecutionDone();
            break;
        }

        case FSMAction::SEND_FEEDBACK: {
            ui.signal(FeedbackType::ACCESS_DENIED);

            DynamicJsonDocument doc(128);
            doc["status"] = "error";
            doc["action"] = "access_denied";
            comm.sendResponse(doc);

            fsm.onExecutionDone();
            break;
        }

        default:
            break;
    }

    // ==== ETATS WAIT_* ====
    switch (fsm.getState()) {

        case SystemState::WAIT_ADD_BADGE: {
            if (rfid.hasNewCard()) {
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);

                bool ok = eeprom.addBadge(uid);
                ui.signal(ok ? FeedbackType::BADGE_ADDED : FeedbackType::ERROR);

                DynamicJsonDocument doc(128);
                doc["status"] = ok ? "success" : "error";
                doc["type"] = "add_badge";
                comm.sendResponse(doc);

                rfid.halt();
                fsm.onExecutionDone();
            }
            break;
        }

        case SystemState::WAIT_REMOVE_BADGE: {
            if (rfid.hasNewCard()) {
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);

                bool ok = eeprom.removeBadge(uid);
                ui.signal(ok ? FeedbackType::BADGE_DELETED : FeedbackType::ERROR);

                DynamicJsonDocument doc(128);
                doc["status"] = ok ? "success" : "error";
                doc["type"] = "remove_badge";
                comm.sendResponse(doc);

                rfid.halt();
                fsm.onExecutionDone();
            }
            break;
        }

        case SystemState::WAIT_RESET_CONFIRM: {
            String cmd;

            if (serialCmdReady) {
                cmd = serialCmd;
                serialCmdReady = false;
                serialCmd = "";
            } else {
                if (!keypad.isCommandReady()) break;
                cmd = keypad.getCommand();
            }

            cmd.replace("#", "");

            DynamicJsonDocument doc(128);
            doc["type"] = "reset";

            if (cmd == "99") {
                eeprom.reset();
                keypad.changeAdminPIN(eeprom.readAdminPIN());
                ui.signal(FeedbackType::RESET_DONE);
                doc["status"] = "success";
                doc["message"] = "EEPROM reset done";
            } else if (cmd == "00") {
                ui.signal(FeedbackType::CANCELLED);
                doc["status"] = "cancelled";
                doc["message"] = "Reset cancelled";
            } else {
                ui.signal(FeedbackType::ERROR);
                doc["status"] = "error";
                doc["message"] = "Invalid reset command";
            }

            comm.sendResponse(doc);
            fsm.onExecutionDone();
            break;
        }

        default:
            break;
    }
}
