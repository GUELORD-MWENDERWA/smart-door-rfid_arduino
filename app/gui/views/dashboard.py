import customtkinter as ctk


class Dashboard(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)

        self.label = ctk.CTkLabel(
            self,
            text="État de la porte : inconnue",
            font=("Arial", 18)
        )
        self.label.pack(pady=10)

    def set_state(self, state: str):
        if state == "opened":
            text = "État de la porte : OUVERTE"
        elif state == "closed":
            text = "État de la porte : FERMÉE"
        else:
            text = f"État de la porte : {state}"

        self.label.configure(text=text)
