import customtkinter as ctk

class BadgeWait(ctk.CTkToplevel):
    def __init__(self, master, on_cancel):
        super().__init__(master)
        self.title("Scan badge")
        self.geometry("300x150")
        self.on_cancel = on_cancel

        ctk.CTkLabel(self, text="Présentez le badge RFID",
                     font=("Arial", 16)).pack(pady=20)

        ctk.CTkButton(self, text="Annuler",
                      command=self.cancel).pack()

    def cancel(self):
        self.on_cancel()
        self.destroy()
