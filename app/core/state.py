import time

class AppState:
    connected = False
    admin_authenticated = False
    admin_timestamp = 0

    @staticmethod
    def set_admin():
        AppState.admin_authenticated = True
        AppState.admin_timestamp = time.time()

    @staticmethod
    def reset_admin():
        AppState.admin_authenticated = False
        AppState.admin_timestamp = 0
