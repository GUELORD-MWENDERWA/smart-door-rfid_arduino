import customtkinter as ctk
from tkinter import messagebox

from core.serial_link import SerialLink
from core.protocol import Protocol
from core.state import AppState

from gui.widgets.status_bar import StatusBar
from gui.widgets.log_panel import LogPanel
from gui.views.dashboard import Dashboard
from gui.views.badge_wait import BadgeWait
from gui.views.admin import AdminDialog


ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")


class DoorApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        # ================= Fenêtre =================
        self.title("Smart Door Control")
        self.geometry("700x600")
        self.resizable(False, False)

        # ================= Serial =================
        self.link = SerialLink()
        self.link.on_message = self.on_message
        self.link.on_status = self.on_status

        self.pending_payload: dict | None = None
        self.badge_window: BadgeWait | None = None
        self._command_in_progress: bool = False

        # ================= Status bar =================
        self.status_bar = StatusBar(self)
        self.status_bar.pack(fill="x", padx=10, pady=5)

        # ================= Connexion =================
        conn = ctk.CTkFrame(self)
        conn.pack(fill="x", padx=10, pady=5)

        self.port_box = ctk.CTkComboBox(
            conn,
            values=SerialLink.list_ports(),
            width=140
        )
        self.port_box.pack(side="left", padx=5)

        ctk.CTkButton(
            conn,
            text="Rafraîchir",
            command=self.refresh_ports,
            width=90
        ).pack(side="left", padx=5)

        ctk.CTkButton(
            conn,
            text="Connecter",
            command=self.connect,
            width=100
        ).pack(side="left", padx=5)

        # ================= Dashboard =================
        self.dashboard = Dashboard(self)
        self.dashboard.pack(fill="x", padx=10, pady=10)

        # ================= Actions ligne 1 =================
        actions1 = ctk.CTkFrame(self)
        actions1.pack(fill="x", padx=10, pady=5)

        self._action_button(actions1, "Ouvrir la porte", Protocol.OPEN_DOOR)
        self._action_button(actions1, "Ajouter badge", Protocol.ADD_BADGE)
        self._action_button(actions1, "Supprimer badge", Protocol.REMOVE_BADGE)
        self._action_button(actions1, "Lister badges", Protocol.LIST_BADGES)
        self._action_button(actions1, "Reset EEPROM", Protocol.RESET_REQUEST)

        # ================= Actions ligne 2 =================
        actions2 = ctk.CTkFrame(self)
        actions2.pack(fill="x", padx=10, pady=5)

        self._action_button(actions2, "Confirmer Reset", Protocol.CONFIRM_RESET)
        self._action_button(actions2, "Annuler Reset", Protocol.CANCEL_RESET)
        self._action_button(actions2, "Changer PIN", Protocol.change_pin("0000"))

        # ================= Log =================
        self.log = LogPanel(self)
        self.log.pack(fill="both", expand=True, padx=10, pady=10)

    # ==================================================
    # UI helpers
    # ==================================================
    def _action_button(self, parent, label, payload):
        ctk.CTkButton(
            parent,
            text=label,
            width=180,
            command=lambda p=payload: self.admin_guard(p)
        ).pack(side="left", padx=5, pady=5)

    def refresh_ports(self):
        self.port_box.configure(values=SerialLink.list_ports())

    # ==================================================
    # Serial
    # ==================================================
    def connect(self):
        port = self.port_box.get().strip()
        if not port:
            messagebox.showwarning("Port", "Veuillez sélectionner un port série")
            return
        self.link.connect(port)

    def on_status(self, connected: bool):
        AppState.connected = connected
        self.status_bar.set_connected(connected)

        if connected:
            self.log.log("[INFO] Arduino connecté")
        else:
            self.log.log("[INFO] Arduino déconnecté")
            AppState.reset_admin()
            self._command_in_progress = False

    # ==================================================
    # Admin guard + gestion commandes en cours
    # ==================================================
    def admin_guard(self, payload: dict):
        if not AppState.connected:
            messagebox.showwarning("Connexion", "Arduino non connecté")
            return

        if self._command_in_progress:
            self.log.log("[INFO] Commande précédente non terminée")
            return

        self._command_in_progress = True
        self.pending_payload = payload

        # Si admin pas encore authentifié et la commande nécessite admin
        if not AppState.admin_authenticated and payload not in (Protocol.CONFIRM_RESET, Protocol.CANCEL_RESET):
            dlg = AdminDialog()
            pin = dlg.get_input()
            if not pin:
                self._command_in_progress = False
                return
            self.link.send(Protocol.admin_auth(pin))
        else:
            self.link.send(payload)
            self._post_command_ui(payload)

    def _post_command_ui(self, payload: dict):
        if payload in (Protocol.ADD_BADGE, Protocol.REMOVE_BADGE):
            self.show_badge_wait()

    def show_badge_wait(self):
        if self.badge_window:
            return
        self.badge_window = BadgeWait(self, on_cancel=self.cancel_badge_wait)

    def cancel_badge_wait(self):
        if self.badge_window:
            self.badge_window.destroy()
            self.badge_window = None

    # ==================================================
    # Messages Arduino
    # ==================================================
    def on_message(self, msg: dict):
        self.log.log(str(msg))

        msg_type = msg.get("type")
        status = msg.get("status")

        # ---- Door state ----
        if msg_type == "door_state":
            self.dashboard.set_state(msg.get("state", "unknown"))

        # ---- Admin auth ----
        elif msg_type == "admin_auth":
            if msg.get("access_granted"):
                AppState.set_admin()
                if self.pending_payload:
                    self.link.send(self.pending_payload)
                    self._post_command_ui(self.pending_payload)
            else:
                AppState.reset_admin()
                messagebox.showerror("Admin", "PIN incorrect")
                self._command_in_progress = False

        # ---- Badge / reset finished ----
        elif msg_type in ("add_badge", "remove_badge", "reset"):
            AppState.reset_admin()
            self.cancel_badge_wait()
            self._command_in_progress = False

        # ---- Command finished ----
        elif msg_type == "command":
            # Mettre à jour état porte si open/close
            if msg.get("action") == "open_door":
                self.dashboard.set_state("opened")
            self._command_in_progress = False
            # Reset admin si nécessaire
            if status in ("success", "error"):
                AppState.reset_admin()

        # ---- Error ----
        if status == "error":
            messagebox.showerror(
                "Erreur Arduino",
                msg.get("message", "Erreur inconnue")
            )
            self._command_in_progress = False

    # ==================================================
    # Clean exit
    # ==================================================
    def destroy(self):
        try:
            self.link.stop()
        except Exception:
            pass
        super().destroy()
