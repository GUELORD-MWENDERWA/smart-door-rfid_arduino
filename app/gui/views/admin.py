import customtkinter as ctk

class AdminDialog(ctk.CTkInputDialog):
    def __init__(self):
        super().__init__(title="Admin", text="Entrez le PIN admin")
