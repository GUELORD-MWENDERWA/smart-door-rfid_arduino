class Protocol:
    OPEN_DOOR = {"cmd": "10"}
    ADD_BADGE = {"cmd": "11"}
    REMOVE_BADGE = {"cmd": "12"}
    LIST_BADGES = {"cmd": "13"}
    RESET_REQUEST = {"cmd": "14"}

    CONFIRM_RESET = {"cmd": "99"}
    CANCEL_RESET = {"cmd": "00"}

    @staticmethod
    def admin_auth(pin: str):
        return {"cmd": pin}

    @staticmethod
    def change_pin(new_pin: str):
        return {"cmd": f"99{new_pin}"}
