import time


class AppState:
    """
    État global de l'application (GUI uniquement)

    - Le CLI ne l'utilise PAS
    - L'auth admin est temporaire et locale au GUI
    """

    connected: bool = False

    # Auth admin côté GUI uniquement
    admin_authenticated: bool = False
    admin_timestamp: float = 0.0

    # Durée de validité session admin (secondes)
    ADMIN_SESSION_TIMEOUT = 30

    # ==================================================
    # ADMIN SESSION
    # ==================================================
    @staticmethod
    def set_admin():
        AppState.admin_authenticated = True
        AppState.admin_timestamp = time.time()

    @staticmethod
    def reset_admin():
        AppState.admin_authenticated = False
        AppState.admin_timestamp = 0.0

    @staticmethod
    def is_admin_valid() -> bool:
        """
        Le GUI considère l'admin valide pendant un court délai
        pour éviter de redemander le PIN à chaque clic.
        """
        if not AppState.admin_authenticated:
            return False

        if time.time() - AppState.admin_timestamp > AppState.ADMIN_SESSION_TIMEOUT:
            AppState.reset_admin()
            return False

        return True

    # ==================================================
    # CONNEXION
    # ==================================================
    @staticmethod
    def set_connected(state: bool):
        AppState.connected = state
        if not state:
            AppState.reset_admin()
