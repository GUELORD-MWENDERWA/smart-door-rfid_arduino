#include <Arduino.h>
#include "fsm/FSMController.h"
#include "rfid/RFIDModule.h"
#include "eeprom/EEPROMStore.h"
#include "keypad/KeypadModule.h"
#include "ui/UIFeedback.h"
#include "relay/RelayController.h"
#include "comm/JsonComm.h"

// ----- PINS -----
const uint8_t SS_PIN = 10;
const uint8_t RST_PIN = 9;
const uint8_t LED_GREEN = 7;
const uint8_t LED_RED   = 6;
const uint8_t BUZZER    = 2;
const uint8_t RELAY_PIN = 8;

// ----- KEYPAD CONFIG 4x4 -----
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS*COLS] = {
  '1','2','3','A',
  '4','5','6','B',
  '7','8','9','C',
  '*','0','#','D'
};
byte rowPins[ROWS] = {A0, A1, A2, A3};
byte colPins[COLS] = {A4, A5, 2, 3};

// ----- MODULES -----
EEPROMStore eeprom;
RFIDModule rfid(SS_PIN, RST_PIN);
RelayController relay(RELAY_PIN, 5000); // 5 secondes
UIFeedback ui(LED_GREEN, LED_RED, BUZZER);
KeypadModule keypad(keys, rowPins, colPins, ROWS, COLS, "123"); // PIN admin par défaut
JsonComm comm(Serial);

// ----- FSM -----
FSMController fsm;

void setup() {
    Serial.begin(9600);

    // Initialisation modules
    eeprom.begin();
    rfid.begin();
    relay.begin();
    ui.begin();
    keypad.begin();
    comm.begin();

    // Signal de démarrage
    ui.signal(FeedbackType::ACCESS_GRANTED);
    delay(200);
    ui.signal(FeedbackType::ACCESS_DENIED);
}

void loop() {
    // Mise à jour modules non bloquante
    keypad.update();
    relay.update();
    rfid.poll();

    // Vérifier si un badge est détecté
    if (rfid.hasNewCard()) {
        fsm.onBadgeDetected();
    }

    // Vérifier si une commande keypad a été saisie
    if (keypad.isCommandReady()) {
        fsm.onCommandDetected();
    }

    // FSM exécute le cœur
    fsm.update();

    // --- Gestion des actions FSM ---
    FSMAction action = fsm.getAction();
    switch(action) {
        case FSMAction::VALIDATE_BADGE: {
            uint8_t uid[EEPROMStore::UID_SIZE];
            rfid.getUID(uid);
            bool valid = eeprom.badgeExists(uid);
            fsm.onBadgeValidationResult(valid);
            fsm.clearAction();
            break;
        }

        case FSMAction::EXECUTE_COMMAND: {
            String cmd = keypad.getCommand();
            keypad.getCommand(); // Clear command après lecture
            keypad.update();     // réinitialiser buffer

            if (cmd == "11#") { // Ajouter badge
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);
                bool result = eeprom.addBadge(uid);
                fsm.onCommandValidationResult(result);
                ui.signal(result ? FeedbackType::BADGE_ADDED : FeedbackType::ERROR);
            } else if (cmd == "12#") { // Supprimer badge
                uint8_t uid[EEPROMStore::UID_SIZE];
                rfid.getUID(uid);
                bool result = eeprom.removeBadge(uid);
                fsm.onCommandValidationResult(result);
                ui.signal(result ? FeedbackType::BADGE_DELETED : FeedbackType::ERROR);
            } else if (cmd == "13#") { // Lister badges
                uint8_t count = eeprom.getBadgeCount();
                comm.sendStatus("success", (String("Badge count: ") + count).c_str());
                fsm.onCommandValidationResult(true);
            } else if (cmd == "14#") { // Reset complet
                eeprom.reset();
                ui.signal(FeedbackType::RESET_DONE);
                fsm.onCommandValidationResult(true);
            } else {
                fsm.onCommandValidationResult(false);
                ui.signal(FeedbackType::ERROR);
            }
            fsm.clearAction();
            break;
        }

        case FSMAction::REQUEST_ADMIN_AUTH: {
            String pin = keypad.getCommand();
            keypad.getCommand(); // Clear command
            bool valid = keypad.checkAdminPIN(pin);
            fsm.onAdminAuthResult(valid);
            fsm.clearAction();
            break;
        }

        case FSMAction::OPEN_DOOR: {
            relay.open();
            fsm.onExecutionDone();
            fsm.clearAction();
            break;
        }

        case FSMAction::SEND_FEEDBACK: {
            // FSM décide du type de feedback
            ui.signal(FeedbackType::ACCESS_GRANTED); // par défaut
            fsm.clearAction();
            break;
        }

        case FSMAction::NONE:
        default:
            break;
    }
}
