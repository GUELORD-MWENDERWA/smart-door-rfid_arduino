import customtkinter as ctk

class LogPanel(ctk.CTkTextbox):
    def __init__(self, master):
        super().__init__(master, height=140)
        self.configure(state="normal")

    def log(self, text: str):
        self.insert("end", text + "\n")
        self.see("end")
