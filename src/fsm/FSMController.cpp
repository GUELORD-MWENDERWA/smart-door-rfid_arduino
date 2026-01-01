#include "FSMController.h"

FSMController::FSMController()
    : state(SystemState::IDLE), action(FSMAction::NONE), lastResult(false)
{ }

/* --- cycle --- */
void FSMController::update() {
    switch (state) {
        case SystemState::IDLE:
            action = FSMAction::NONE;
            // en attente d'un badge ou d'une commande
            break;

        case SystemState::RFID_READ:
            action = FSMAction::VALIDATE_BADGE;
            // attendre onBadgeValidationResult() pour continuer
            break;

        case SystemState::INPUT_CMD:
            action = FSMAction::EXECUTE_COMMAND;
            // attendre onCommandValidationResult() pour continuer
            break;

        case SystemState::AUTH_ADMIN:
            action = FSMAction::REQUEST_ADMIN_AUTH;
            // attendre onAdminAuthResult() pour continuer
            break;

        case SystemState::VALIDATE:
            // badge validé
            if (lastResult) {
                state = SystemState::EXECUTE;
            } else {
                state = SystemState::FEEDBACK;
            }
            action = FSMAction::SEND_FEEDBACK;
            break;

        case SystemState::VALIDATE_CMD:
            // commande validée
            if (lastResult) {
                state = SystemState::EXECUTE;
            } else {
                state = SystemState::FEEDBACK;
            }
            action = FSMAction::SEND_FEEDBACK;
            break;

        case SystemState::EXECUTE:
            action = FSMAction::OPEN_DOOR;
            // attendre onExecutionDone() pour revenir à FEEDBACK
            break;

        case SystemState::FEEDBACK:
            action = FSMAction::SEND_FEEDBACK;
            state = SystemState::IDLE;
            break;
    }
}

/* --- événements externes --- */
void FSMController::onBadgeDetected() {
    state = SystemState::RFID_READ;
}

void FSMController::onCommandDetected() {
    state = SystemState::INPUT_CMD;
}

void FSMController::onAdminAuthResult(bool success) {
    lastResult = success;
    state = success ? SystemState::EXECUTE : SystemState::FEEDBACK;
}

void FSMController::onBadgeValidationResult(bool success) {
    lastResult = success;
    state = SystemState::VALIDATE;
}

void FSMController::onCommandValidationResult(bool success) {
    lastResult = success;
    state = SystemState::VALIDATE_CMD;
}

void FSMController::onExecutionDone() {
    state = SystemState::FEEDBACK;
}

/* --- sorties FSM --- */
FSMAction FSMController::getAction() const {
    return action;
}

SystemState FSMController::getState() const {
    return state;
}

void FSMController::clearAction() {
    action = FSMAction::NONE;
}
