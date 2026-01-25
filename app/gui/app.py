import customtkinter as ctk
from tkinter import messagebox, simpledialog

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
        self.geometry("720x600")  # largeur légèrement augmentée
        self.resizable(False, False)

        # ================= Serial =================
        self.link = SerialLink()
        self.link.on_message = self.on_message
        self.link.on_status = self.on_status

        self.pending_payload = None
        self.badge_window = None
        self._command_in_progress = False

        # ================= Status bar =================
        self.status_bar = StatusBar(self)
        self.status_bar.pack(fill="x", padx=10, pady=5)

        # ================= Connexion =================
        conn = ctk.CTkFrame(self)
        conn.pack(fill="x", padx=10, pady=5)

        self.port_box = ctk.CTkComboBox(
            conn,
            values=SerialLink.list_ports(),
            width=150
        )
        self.port_box.pack(side="left", padx=5)

        ctk.CTkButton(
            conn, text="Rafraîchir",
            command=self.refresh_ports, width=90
        ).pack(side="left", padx=5)

        ctk.CTkButton(
            conn, text="Connecter",
            command=self.connect, width=100
        ).pack(side="left", padx=5)

        # ================= Dashboard =================
        self.dashboard = Dashboard(self)
        self.dashboard.pack(fill="x", padx=10, pady=10)

        # ================= Actions ligne 1 =================
        actions1 = ctk.CTkFrame(self)
        actions1.pack(fill="x", padx=10, pady=5)

        self._action_button(actions1, "Ouvrir la porte", self.open_door)
        self._action_button(actions1, "Ajouter badge", lambda: self.secure_cmd(Protocol.ADD_BADGE))
        self._action_button(actions1, "Supprimer badge", lambda: self.secure_cmd(Protocol.REMOVE_BADGE))

        # ================= Actions ligne 2 =================
        actions2 = ctk.CTkFrame(self)
        actions2.pack(fill="x", padx=10, pady=5)

        self._action_button(actions2, "Lister badges", lambda: self.secure_cmd(Protocol.LIST_BADGES))
        self._action_button(actions2, "Reset EEPROM", self.reset_eeprom)
        self._action_button(actions2, "Changer PIN", self.change_pin)

        # ================= Log =================
        self.log = LogPanel(self)
        self.log.pack(fill="both", expand=True, padx=10, pady=10)

    # ==================================================
    # UI helpers
    # ==================================================
    def _action_button(self, parent, label, callback):
        ctk.CTkButton(
            parent, text=label,
            width=200, command=callback
        ).pack(side="left", padx=6, pady=4)

    def refresh_ports(self):
        self.port_box.configure(values=SerialLink.list_ports())

    # ==================================================
    # Connexion série
    # ==================================================
    def connect(self):
        port = self.port_box.get().strip()
        if not port:
            messagebox.showwarning("Port", "Veuillez sélectionner un port")
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
    # Commandes sécurisées (ADMIN)
    # ==================================================
    def secure_cmd(self, payload: dict):
        if not self._begin_command():
            return
        self.pending_payload = payload
        self._ensure_admin_then_send()

    def open_door(self):
        if not self._begin_command():
            return
        self.pending_payload = Protocol.OPEN_DOOR
        self._ensure_admin_then_send()

    def reset_eeprom(self):
        if not self._begin_command():
            return

        if not messagebox.askyesno(
            "Reset EEPROM",
            "Voulez-vous vraiment réinitialiser l'EEPROM ?"
        ):
            self._command_in_progress = False
            return

        self.pending_payload = Protocol.RESET_REQUEST
        self._ensure_admin_then_send()

    def change_pin(self):
        if not self._begin_command():
            return

        new_pin = simpledialog.askstring(
            "Changer PIN",
            "Entrez le nouveau PIN admin :",
            show="*"
        )

        if not new_pin:
            self._command_in_progress = False
            return

        self.pending_payload = Protocol.change_pin(new_pin)
        self._ensure_admin_then_send()

    def _begin_command(self) -> bool:
        if not AppState.connected:
            messagebox.showwarning("Connexion", "Arduino non connecté")
            return False
        if self._command_in_progress:
            self.log.log("[INFO] Commande déjà en cours")
            return False
        self._command_in_progress = True
        return True

    def _ensure_admin_then_send(self):
        if not AppState.admin_authenticated:
            dlg = AdminDialog()
            pin = dlg.get_input()
            if not pin:
                self._command_in_progress = False
                return
            self.link.send(Protocol.admin_auth(pin))
        else:
            self.link.send(self.pending_payload)
            self._post_command_ui()

    def _post_command_ui(self):
        if self.pending_payload in (Protocol.ADD_BADGE, Protocol.REMOVE_BADGE):
            self.show_badge_wait()

    # ==================================================
    # Badge wait
    # ==================================================
    def show_badge_wait(self):
        if not self.badge_window:
            self.badge_window = BadgeWait(self, on_cancel=self.cancel_badge_wait)

    def cancel_badge_wait(self):
        if self.badge_window:
            self.badge_window.destroy()
            self.badge_window = None

    # ==================================================
    # Réponses Arduino
    # ==================================================
    def on_message(self, msg: dict):
        self.log.log(str(msg))

        t = msg.get("type")
        status = msg.get("status")

        # --- Etat porte ---
        if t == "door_state":
            self.dashboard.set_state(msg.get("state"))

        # --- Auth admin ---
        elif t == "admin_auth":
            if msg.get("access_granted"):
                AppState.set_admin()
                self.link.send(self.pending_payload)
                self._post_command_ui()
            else:
                messagebox.showerror("Admin", "PIN incorrect")

        # --- Fin badge / reset ---
        elif t in ("add_badge", "remove_badge", "reset"):
            self.cancel_badge_wait()

        # --- FIN DE COMMANDE (GARANTIE) ---
        if t != "admin_auth":
            self._command_in_progress = False

        if status == "error":
            messagebox.showerror(
                "Erreur Arduino",
                msg.get("message", "Erreur inconnue")
            )

    # ==================================================
    # Fermeture propre
    # ==================================================
    def destroy(self):
        try:
            self.link.stop()
        except Exception:
            pass
        super().destroy()
