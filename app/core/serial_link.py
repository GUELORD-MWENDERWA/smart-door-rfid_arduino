import serial
import serial.tools.list_ports
import threading
import json
import time
from typing import Optional, Callable


class SerialLink:
    def __init__(self, baudrate: int = 115200):
        self.port: Optional[str] = None
        self.baudrate = baudrate
        self.ser: Optional[serial.Serial] = None

        self.running = False
        self.thread: Optional[threading.Thread] = None

        self.on_message: Optional[Callable[[dict], None]] = None
        self.on_status: Optional[Callable[[bool], None]] = None

    # ==================================================
    # LISTE DES PORTS (UTILISÉE UNIQUEMENT POUR LE GUI)
    # ==================================================
    @staticmethod
    def list_ports() -> list[str]:
        return [p.device for p in serial.tools.list_ports.comports()]

    # ==================================================
    # CONNEXION / RECONNEXION ROBUSTE 
    # ==================================================
    def connect(self, port: str):
        self.port = port

        if self.running:
            return

        self.running = True
        self.thread = threading.Thread(
            target=self._worker,
            daemon=True
        )
        self.thread.start()

    def _worker(self):
        while self.running:
            # ---- Tentative connexion ----
            if not self.ser:
                try:
                    self.ser = serial.Serial(
                        self.port,
                        self.baudrate,
                        timeout=0.1,
                        write_timeout=0.1
                    )

                    time.sleep(2)  # reset Arduino

                    if self.on_status:
                        self.on_status(True)

                except serial.SerialException:
                    self.ser = None
                    if self.on_status:
                        self.on_status(False)
                    time.sleep(1)
                    continue

            # ---- Lecture ----
            try:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode(
                        "utf-8",
                        errors="ignore"
                    ).strip()

                    if not line:
                        continue

                    try:
                        obj = json.loads(line)
                        if self.on_message:
                            self.on_message(obj)
                    except json.JSONDecodeError:
                        pass

            except serial.SerialException:
                try:
                    self.ser.close()
                except:
                    pass
                self.ser = None
                if self.on_status:
                    self.on_status(False)
                time.sleep(1)

    # ==================================================
    # ENVOI 
    # ==================================================
    def send(self, payload: dict):
        if not self.ser or not self.ser.is_open:
            raise RuntimeError("Port série non connecté")

        data = json.dumps(payload) + "\n"
        self.ser.write(data.encode("utf-8"))
        self.ser.flush()

    # ==================================================
    # ARRÊT PROPRE
    # ==================================================
    def stop(self):
        self.running = False

        if self.ser:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None

        if self.on_status:
            self.on_status(False)
