#include "FSMController.h"

#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)
#endif

FSMController::FSMController()
    : state(SystemState::IDLE),
      action(FSMAction::NONE),
      lastResult(false),
      lastState(SystemState::IDLE),
      lastAction(FSMAction::NONE)
{
}

void FSMController::update() {
    // For now FSM update is lightweight: could be extended with timers
    // Keep lastState/action for debug
    if (state != lastState || action != lastAction) {
        DEBUG_PRINT(F("[FSM] State: "));
        DEBUG_PRINTLN(stateToStr(state));
        DEBUG_PRINT(F("[FSM] Action: "));
        DEBUG_PRINTLN(actionToStr(action));
        lastState = state;
        lastAction = action;
    }
}

void FSMController::onBadgeDetected() {
    // When a badge is detected, request validation
    action = FSMAction::VALIDATE_BADGE;
}

void FSMController::onCommandDetected() {
    // Trigger admin auth request or execution depending on state
    action = FSMAction::REQUEST_ADMIN_AUTH; // default; higher level (main) will use action EXECUTE_COMMAND when appropriate
    // In existing main.cpp the control flow sets action differently; keep compatible by letting main set exact action where necessary
    // We set REQUEST_ADMIN_AUTH as a neutral signal
}

void FSMController::onAdminAuthResult(bool success) {
    lastResult = success;
    if (success) {
        action = FSMAction::EXECUTE_COMMAND;
    } else {
        action = FSMAction::SEND_FEEDBACK;
    }
}

void FSMController::onBadgeValidationResult(bool success) {
    lastResult = success;
    if (success) {
        action = FSMAction::OPEN_DOOR;
    } else {
        action = FSMAction::SEND_FEEDBACK;
    }
}

void FSMController::onCommandValidationResult(bool success) {
    lastResult = success;
    if (success) {
        action = FSMAction::EXECUTE_COMMAND;
    } else {
        action = FSMAction::SEND_FEEDBACK;
    }
}

void FSMController::onExecutionDone() {
    action = FSMAction::NONE;
    state = SystemState::IDLE;
}

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

/* ----- debug helpers ----- */

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