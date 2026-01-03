#ifndef FSM_CONTROLLER_H
#define FSM_CONTROLLER_H

#include <Arduino.h>

/* ===== États du système ===== */
enum class SystemState : uint8_t {
    IDLE,
    RFID_READ,
    INPUT_CMD,
    VALIDATE,
    VALIDATE_CMD,
    EXECUTE,
    FEEDBACK,

    WAIT_ADD_BADGE,
    WAIT_REMOVE_BADGE,
    WAIT_RESET_CONFIRM
};

/* ===== Actions FSM ===== */
enum class FSMAction : uint8_t {
    NONE,
    VALIDATE_BADGE,
    REQUEST_ADMIN_AUTH,
    EXECUTE_COMMAND,
    OPEN_DOOR,
    SEND_FEEDBACK
};

class FSMController {
public:
    FSMController();

    void update();

    void onBadgeDetected();
    void onCommandDetected();
    void onAdminAuthResult(bool success);
    void onBadgeValidationResult(bool success);
    void onCommandValidationResult(bool success);
    void onExecutionDone();

    FSMAction getAction() const;
    SystemState getState() const;

    void clearAction();
    void setState(SystemState newState);

private:
    SystemState state;
    FSMAction action;
    bool lastResult;

    SystemState lastState;
    FSMAction lastAction;

    const char* stateToStr(SystemState s);
    const char* actionToStr(FSMAction a);
};

#endif
