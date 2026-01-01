#ifndef FSM_CONTROLLER_H
#define FSM_CONTROLLER_H

#include <Arduino.h>

/* --- États système --- */
enum class SystemState : uint8_t {
    IDLE,           // Attente d'événements (badge ou commande)
    RFID_READ,      // Lecture d'un badge RFID
    INPUT_CMD,      // Lecture d'une commande Keypad
    AUTH_ADMIN,     // Attente validation PIN admin
    VALIDATE,       // Validation badge
    VALIDATE_CMD,   // Validation commande
    EXECUTE,        // Exécution action (porte, CRUD)
    FEEDBACK        // Feedback utilisateur
};

/* --- Actions FSM --- */
enum class FSMAction : uint8_t {
    NONE,               // Aucune action
    VALIDATE_BADGE,     // Vérifier badge RFID
    REQUEST_ADMIN_AUTH, // Demander authentification admin
    EXECUTE_COMMAND,    // Exécuter commande Keypad
    OPEN_DOOR,          // Ouvrir la porte / relais
    SEND_FEEDBACK       // Signaler résultat via LED/Buzzer/JSON
};

/* --- Contrôleur FSM --- */
class FSMController {
public:
    FSMController();

    /* --- Cycle principal --- */
    void update();

    /* --- Événements externes --- */
    void onBadgeDetected();                 // Badge détecté
    void onCommandDetected();               // Commande saisie
    void onAdminAuthResult(bool success);   // Résultat PIN admin
    void onBadgeValidationResult(bool success);  // Résultat validation badge
    void onCommandValidationResult(bool success); // Résultat validation commande
    void onExecutionDone();                 // Fin d’exécution

    /* --- Sorties FSM --- */
    FSMAction getAction() const;            // Action à exécuter
    SystemState getState() const;           // État courant
    void clearAction();                     // Réinitialiser action après exécution

private:
    SystemState state;  // État courant
    FSMAction action;   // Action courante
    bool lastResult;    // Dernier résultat (validation)
};

#endif
