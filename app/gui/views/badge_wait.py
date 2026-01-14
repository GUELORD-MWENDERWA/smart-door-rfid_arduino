import customtkinter as ctk


class BadgeWait(ctk.CTkToplevel):
    def __init__(self, parent, on_cancel=None):
        super().__init__(parent)

        self.on_cancel = on_cancel

        self.title("Badge RFID")
        self.geometry("320x160")
        self.resizable(False, False)
        self.grab_set()

        ctk.CTkLabel(
            self,
            text="Présentez un badge RFID",
            font=("Arial", 16)
        ).pack(pady=20)

        ctk.CTkButton(
            self,
            text="Annuler",
            command=self._cancel,
            width=120
        ).pack(pady=10)

    def _cancel(self):
        if self.on_cancel:
            self.on_cancel()
        self.destroy()
