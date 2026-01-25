# core/protocol.py

class Protocol:
    """
    Définition STRICTE des commandes série acceptées par l'Arduino.
    Ce module ne contient AUCUNE logique métier.
    """

    # ==================================================
    # COMMANDES ADMIN (nécessitent un PIN AVANT)
    # ==================================================
    OPEN_DOOR = {"cmd": "10"}
    ADD_BADGE = {"cmd": "11"}
    REMOVE_BADGE = {"cmd": "12"}
    LIST_BADGES = {"cmd": "13"}
    RESET_REQUEST = {"cmd": "14"}

    # ==================================================
    # SOUS-COMMANDES (utilisées UNIQUEMENT après RESET_REQUEST)
    # OU APRÈS AUTH ADMIN VALIDE
    # ==================================================
    CONFIRM_RESET = {"cmd": "99"}
    CANCEL_RESET = {"cmd": "00"}
   
    # ==================================================
    # AUTHENTIFICATION ADMIN
    # ==================================================
    @staticmethod
    def admin_auth(pin: str) -> dict:
        """
        Envoi du PIN admin.
        Exemple : {"cmd": "123"}
        """
        return {"cmd": pin}

    # ==================================================
    # CHANGEMENT PIN ADMIN
    # ==================================================
    @staticmethod
    def change_pin(new_pin: str) -> dict:
        """
        Commande de changement de PIN admin.
        Séquence attendue par l'Arduino :
            1. PIN admin   -> {"cmd": "123"}
            2. Change PIN  -> {"cmd": "99<new_pin>"}
        """
        return {"cmd": f"99{new_pin}"}
