import customtkinter as ctk

class StatusBar(ctk.CTkLabel):
    def __init__(self, master):
        super().__init__(master, text="Déconnecté", text_color="red")

    def set_connected(self, ok: bool):
        self.configure(
            text="Connecté" if ok else "Déconnecté",
            text_color="green" if ok else "red"
        )
