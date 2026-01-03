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

    if (keypad.isCommandReady() && fsm.getState() == SystemState::IDLE) {
        fsm.onCommandDetected();
    }

    if (rfid.poll() && rfid.hasNewCard()) {
        fsm.onBadgeDetected();
    }

    fsm.update();

    switch (fsm.getAction()) {

        case FSMAction::VALIDATE_BADGE: {
            uint8_t uid[EEPROMStore::UID_SIZE];
            rfid.getUID(uid);
            bool ok = eeprom.badgeExists(uid);
            fsm.onBadgeValidationResult(ok);
            fsm.clearAction();
            break;
        }

        case FSMAction::REQUEST_ADMIN_AUTH: {
            if (!keypad.isCommandReady()) break;

            String storedPin = eeprom.readAdminPIN();
            bool ok = keypad.checkAdminPIN(keypad.getCommand());
            fsm.onAdminAuthResult(ok);

            if (!ok) {
                ui.signal(keypad.isLocked()
                        ? FeedbackType::LOCKED
                        : FeedbackType::ERROR);
            }

            fsm.clearAction();
            break;
        }

        case FSMAction::EXECUTE_COMMAND: {
            if (!keypad.isCommandReady()) break;

            String cmd = keypad.getCommand();
            cmd.replace("#", "");

            if (cmd == "11") {
                ui.signal(FeedbackType::SCAN_BADGE);
                fsm.setState(SystemState::WAIT_ADD_BADGE);
                fsm.clearAction();

            } else if (cmd == "12") {
                ui.signal(FeedbackType::SCAN_BADGE);
                fsm.setState(SystemState::WAIT_REMOVE_BADGE);
                fsm.clearAction();

            } else if (cmd == "13") {
                DynamicJsonDocument doc(512);
                doc["status"] = "success";
                doc["total_badges"] = eeprom.getBadgeCount();

                JsonArray badges = doc["badges"].to<JsonArray>();
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

                comm.sendResponse(doc);
                fsm.onExecutionDone();

            } else if (cmd == "14") {
                ui.signal(FeedbackType::CONFIRM_RESET);
                fsm.setState(SystemState::WAIT_RESET_CONFIRM);
                fsm.clearAction();

            } else if (cmd.startsWith("99")) {
                String newPin = cmd.substring(2);
                if (newPin.length() >= 3 && newPin.length() <= 6) {
                    if (keypad.changeAdminPIN(newPin)) {
                        eeprom.writeAdminPIN(newPin);
                        ui.signal(FeedbackType::ACCESS_GRANTED);
                        comm.sendStatus("success", "Admin PIN updated");
                    } else {
                        ui.signal(FeedbackType::ERROR);
                        comm.sendStatus("error", "Invalid PIN");
                    }
                } else {
                    ui.signal(FeedbackType::ERROR);
                    comm.sendStatus("error", "PIN length must be 3-6 digits");
                }
                fsm.onExecutionDone();

            } else {
                ui.signal(FeedbackType::ERROR);
                fsm.onExecutionDone();
            }
            break;
        }

        case FSMAction::OPEN_DOOR: {
            relay.open();
            ui.signal(FeedbackType::ACCESS_GRANTED);
            fsm.onExecutionDone();
            break;
        }

        case FSMAction::SEND_FEEDBACK: {
            ui.signal(FeedbackType::ACCESS_DENIED);
            fsm.onExecutionDone();
            break;
        }

        default:
            break;
    }

    switch (fsm.getState()) {

        case SystemState::WAIT_ADD_BADGE: {
            if (rfid.hasNewCard()) {
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);
                ui.signal(eeprom.addBadge(uid)
                          ? FeedbackType::BADGE_ADDED
                          : FeedbackType::ERROR);
                rfid.halt();
                fsm.onExecutionDone();
            }
            break;
        }

        case SystemState::WAIT_REMOVE_BADGE: {
            if (rfid.hasNewCard()) {
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);
                ui.signal(eeprom.removeBadge(uid)
                          ? FeedbackType::BADGE_DELETED
                          : FeedbackType::ERROR);
                rfid.halt();
                fsm.onExecutionDone();
            }
            break;
        }

        case SystemState::WAIT_RESET_CONFIRM: {
            if (!keypad.isCommandReady()) break;

            String cmd = keypad.getCommand();
            cmd.replace("#", "");

            if (cmd == "99") {
                eeprom.reset();
                ui.signal(FeedbackType::RESET_DONE);
            } else if (cmd == "00") {
                ui.signal(FeedbackType::CANCELLED);
            } else {
                ui.signal(FeedbackType::ERROR);
            }

            fsm.onExecutionDone();
            break;
        }

        default:
            break;
    }
}
