import customtkinter as ctk


class StatusBar(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)

        self.label = ctk.CTkLabel(
            self,
            text="● Déconnecté",
            text_color="red"
        )
        self.label.pack(side="left", padx=10)

    def set_connected(self, connected: bool):
        if connected:
            self.label.configure(
                text="● Connecté",
                text_color="green"
            )
        else:
            self.label.configure(
                text="● Déconnecté",
                text_color="red"
            )
