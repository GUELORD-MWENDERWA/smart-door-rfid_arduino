import customtkinter as ctk


class AdminDialog(ctk.CTkToplevel):
    def __init__(self):
        super().__init__()

        self.title("Authentification admin")
        self.geometry("300x160")
        self.resizable(False, False)
        self.grab_set()

        self._value = None

        ctk.CTkLabel(
            self,
            text="Entrez le PIN administrateur",
            font=("Arial", 14)
        ).pack(pady=10)

        self.entry = ctk.CTkEntry(
            self,
            show="*",
            width=180
        )
        self.entry.pack(pady=5)
        self.entry.focus()

        ctk.CTkButton(
            self,
            text="Valider",
            command=self._validate,
            width=120
        ).pack(pady=10)

        self.bind("<Return>", lambda _: self._validate())

    def _validate(self):
        value = self.entry.get().strip()
        if value:
            self._value = value
        self.destroy()

    def get_input(self) -> str | None:
        self.wait_window()
        return self._value
