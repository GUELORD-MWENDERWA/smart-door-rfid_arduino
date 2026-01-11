import customtkinter as ctk

class Dashboard(ctk.CTkFrame):
    def __init__(self, master):
        super().__init__(master)

        self.door_state = ctk.CTkLabel(
            self, text="PORTE : INCONNUE", font=("Arial", 20)
        )
        self.door_state.pack(pady=20)

    def set_state(self, state: str):
        self.door_state.configure(text=f"PORTE : {state.upper()}")
