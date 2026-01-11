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

        self.pending_payload = None
        self.badge_window = None

        # ================= Status bar =================
        self.status_bar = StatusBar(self)
        self.status_bar.pack(fill="x", padx=10, pady=5)

        # ================= Connexion =================
        conn_frame = ctk.CTkFrame(self)
        conn_frame.pack(fill="x", padx=10, pady=5)

        self.port_box = ctk.CTkComboBox(
            conn_frame,
            values=SerialLink.list_ports(),
            width=140
        )
        self.port_box.pack(side="left", padx=5)

        ctk.CTkButton(
            conn_frame,
            text="Rafraîchir",
            command=self.refresh_ports,
            width=90
        ).pack(side="left", padx=5)

        ctk.CTkButton(
            conn_frame,
            text="Connecter",
            command=self.connect,
            width=100
        ).pack(side="left", padx=5)

        # ================= Dashboard =================
        self.dashboard = Dashboard(self)
        self.dashboard.pack(fill="x", padx=10, pady=10)

        # ================= Actions =================
        actions = ctk.CTkFrame(self)
        actions.pack(fill="x", padx=10, pady=5)

        self._action_button(actions, "Ouvrir la porte", Protocol.OPEN_DOOR)
        self._action_button(actions, "Ajouter badge", Protocol.ADD_BADGE)
        self._action_button(actions, "Supprimer badge", Protocol.REMOVE_BADGE)
        self._action_button(actions, "Lister badges", Protocol.LIST_BADGES)
        self._action_button(actions, "Reset EEPROM", Protocol.RESET_REQUEST)

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
            command=lambda p=payload: self.admin_guard(p),
            width=180
        ).pack(side="left", padx=5, pady=5)

    def refresh_ports(self):
        self.port_box.configure(values=SerialLink.list_ports())

    # ==================================================
    # Serial control
    # ==================================================
    def connect(self):
        port = self.port_box.get()
        if not port:
            messagebox.showwarning("Port", "Veuillez sélectionner un port série")
            return
        self.link.connect(port)

    def on_status(self, connected: bool):
        AppState.connected = connected
        self.status_bar.set_connected(connected)

        if connected:
            self.log.log("[INFO] Connecté au port série")
        else:
            self.log.log("[INFO] Déconnecté du port série")

    # ==================================================
    # Admin guard
    # ==================================================
    def admin_guard(self, payload: dict):
        if not AppState.admin_authenticated:
            dlg = AdminDialog()
            pin = dlg.get_input()

            if not pin:
                return

            self.pending_payload = payload
            self.link.send(Protocol.admin_auth(pin))
        else:
            self.link.send(payload)

            if payload in (Protocol.ADD_BADGE, Protocol.REMOVE_BADGE):
                self.show_badge_wait()

    def show_badge_wait(self):
        if self.badge_window:
            return

        self.badge_window = BadgeWait(
            self,
            on_cancel=self.cancel_badge_wait
        )

    def cancel_badge_wait(self):
        self.badge_window = None

    # ==================================================
    # Messages Arduino
    # ==================================================
    def on_message(self, msg: dict):
        self.log.log(str(msg))

        # ---- Door state ----
        if msg.get("type") == "door_state":
            self.dashboard.set_state(msg.get("state", "unknown"))

        # ---- Admin auth ----
        if msg.get("type") == "admin_auth":
            if msg.get("access_granted"):
                AppState.set_admin()
                if self.pending_payload:
                    self.link.send(self.pending_payload)

                    if self.pending_payload in (
                        Protocol.ADD_BADGE,
                        Protocol.REMOVE_BADGE
                    ):
                        self.show_badge_wait()
            else:
                messagebox.showerror("Erreur", "PIN admin incorrect")

        # ---- Badge done / reset ----
        if msg.get("type") in ("add_badge", "remove_badge", "reset"):
            AppState.reset_admin()
            if self.badge_window:
                self.badge_window.destroy()
                self.badge_window = None

        # ---- Error ----
        if msg.get("status") == "error":
            messagebox.showerror(
                "Erreur Arduino",
                msg.get("message", "Erreur inconnue")
            )

    # ==================================================
    # Clean exit
    # ==================================================
    def destroy(self):
        try:
            self.link.stop()
        except:
            pass
        super().destroy()
