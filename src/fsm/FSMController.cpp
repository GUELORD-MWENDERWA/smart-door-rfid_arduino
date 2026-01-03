#include "FSMController.h"

FSMController::FSMController()
    : state(SystemState::IDLE),
      action(FSMAction::NONE),
      lastResult(false),
      lastState(SystemState::IDLE),
      lastAction(FSMAction::NONE)
{
    Serial.println(F("[FSM] Init -> IDLE"));
}

void FSMController::update() {

    if (state != lastState || action != lastAction) {
        Serial.print(F("[FSM] State="));
        Serial.print(stateToStr(state));
        Serial.print(F(" | Action="));
        Serial.println(actionToStr(action));
        lastState = state;
        lastAction = action;
    }

    if (action != FSMAction::NONE) return;

    switch (state) {

        case SystemState::RFID_READ:
            action = FSMAction::VALIDATE_BADGE;
            break;

        case SystemState::INPUT_CMD:
            action = FSMAction::REQUEST_ADMIN_AUTH;
            break;

        case SystemState::VALIDATE:
            action = lastResult ? FSMAction::OPEN_DOOR
                                : FSMAction::SEND_FEEDBACK;
            state = SystemState::FEEDBACK;
            break;

        case SystemState::VALIDATE_CMD:
            action = lastResult ? FSMAction::EXECUTE_COMMAND
                                : FSMAction::SEND_FEEDBACK;
            state = SystemState::EXECUTE;
            break;

        case SystemState::FEEDBACK:
            action = FSMAction::SEND_FEEDBACK;
            break;

        default:
            break;
    }
}

/* ===== EVENTS ===== */

void FSMController::onBadgeDetected() {
    if (state == SystemState::IDLE) {
        state = SystemState::RFID_READ;
    }
}

void FSMController::onCommandDetected() {
    if (state == SystemState::IDLE) {
        state = SystemState::INPUT_CMD;
    }
}

void FSMController::onBadgeValidationResult(bool success) {
    lastResult = success;
    state = SystemState::VALIDATE;
}

void FSMController::onAdminAuthResult(bool success) {
    lastResult = success;
    state = SystemState::VALIDATE_CMD;
}

void FSMController::onCommandValidationResult(bool success) {
    lastResult = success;
    state = SystemState::EXECUTE;
}

void FSMController::onExecutionDone() {
    state = SystemState::IDLE;
    action = FSMAction::NONE;
}

/* ===== OUTPUTS ===== */

FSMAction FSMController::getAction() const {
    return action;
}

SystemState FSMController::getState() const {
    return state;
}

void FSMController::clearAction() {
    action = FSMAction::NONE;
}

void FSMController::setState(SystemState newState) {
    state = newState;
}

/* ===== DEBUG ===== */

const char* FSMController::stateToStr(SystemState s) {
    switch (s) {
        case SystemState::IDLE: return "IDLE";
        case SystemState::RFID_READ: return "RFID_READ";
        case SystemState::INPUT_CMD: return "INPUT_CMD";
        case SystemState::VALIDATE: return "VALIDATE";
        case SystemState::VALIDATE_CMD: return "VALIDATE_CMD";
        case SystemState::EXECUTE: return "EXECUTE";
        case SystemState::FEEDBACK: return "FEEDBACK";
        case SystemState::WAIT_ADD_BADGE: return "WAIT_ADD_BADGE";
        case SystemState::WAIT_REMOVE_BADGE: return "WAIT_REMOVE_BADGE";
        case SystemState::WAIT_RESET_CONFIRM: return "WAIT_RESET_CONFIRM";
        default: return "UNKNOWN";
    }
}

const char* FSMController::actionToStr(FSMAction a) {
    switch (a) {
        case FSMAction::NONE: return "NONE";
        case FSMAction::VALIDATE_BADGE: return "VALIDATE_BADGE";
        case FSMAction::REQUEST_ADMIN_AUTH: return "REQUEST_ADMIN_AUTH";
        case FSMAction::EXECUTE_COMMAND: return "EXECUTE_COMMAND";
        case FSMAction::OPEN_DOOR: return "OPEN_DOOR";
        case FSMAction::SEND_FEEDBACK: return "SEND_FEEDBACK";
        default: return "UNKNOWN";
    }
}
