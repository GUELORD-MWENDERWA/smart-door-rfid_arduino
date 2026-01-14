import customtkinter as ctk
from datetime import datetime


class LogPanel(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent)

        self.text = ctk.CTkTextbox(
            self,
            height=220,
            wrap="word"
        )
        self.text.pack(fill="both", expand=True)

        self.text.configure(state="disabled")

    def log(self, message: str):
        timestamp = datetime.now().strftime("%H:%M:%S")
        line = f"[{timestamp}] {message}\n"

        self.text.configure(state="normal")
        self.text.insert("end", line)
        self.text.see("end")
        self.text.configure(state="disabled")
