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

        # Callbacks
        self.on_message: Optional[Callable[[dict], None]] = None
        self.on_status: Optional[Callable[[bool], None]] = None

        # Synchronisation connexion réelle
        self._connected_event = threading.Event()
        self._lock = threading.Lock()

    # ==================================================
    # LISTE DES PORTS (CLI / GUI)
    # ==================================================
    @staticmethod
    def list_ports() -> list[str]:
        return [p.device for p in serial.tools.list_ports.comports()]

    # ==================================================
    # CONNEXION
    # ==================================================
    def connect(self, port: str):
        if self.running:
            return

        self.port = port
        self.running = True
        self.thread = threading.Thread(
            target=self._worker,
            daemon=True
        )
        self.thread.start()

    def wait_connected(self, timeout: float = 5.0) -> bool:
        """
        Utilisé par le CLI pour bloquer jusqu'à ce que l'Arduino
        soit réellement prêt après reset.
        """
        return self._connected_event.wait(timeout)

    # ==================================================
    # THREAD PRINCIPAL
    # ==================================================
    def _worker(self):
        while self.running:

            # ---------- Connexion ----------
            if not self.ser:
                try:
                    self.ser = serial.Serial(
                        self.port,
                        self.baudrate,
                        timeout=0.05,
                        write_timeout=0.1
                    )

                    # Reset Arduino
                    time.sleep(2.2)

                    self._connected_event.set()
                    if self.on_status:
                        self.on_status(True)

                except serial.SerialException:
                    self.ser = None
                    self._connected_event.clear()
                    if self.on_status:
                        self.on_status(False)
                    time.sleep(1)
                    continue

            # ---------- Lecture ----------
            try:
                while self.ser and self.ser.in_waiting:
                    line = self.ser.readline().decode(
                        "utf-8", errors="ignore"
                    ).strip()

                    if not line:
                        continue

                    try:
                        obj = json.loads(line)
                        if self.on_message:
                            self.on_message(obj)
                    except json.JSONDecodeError:
                        # Ignore bruit série
                        pass

            except serial.SerialException:
                self._handle_disconnect()

    # ==================================================
    # ENVOI (THREAD-SAFE)
    # ==================================================
    def send(self, payload: dict):
        if not self._connected_event.is_set():
            raise RuntimeError("Arduino non connecté")

        data = json.dumps(payload) + "\n"

        with self._lock:
            try:
                self.ser.write(data.encode("utf-8"))
                self.ser.flush()
            except serial.SerialException:
                self._handle_disconnect()
                raise RuntimeError("Erreur d'envoi série")

    # ==================================================
    # GESTION DECONNEXION
    # ==================================================
    def _handle_disconnect(self):
        self._connected_event.clear()

        if self.ser:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None

        if self.on_status:
            self.on_status(False)

        time.sleep(1)

    # ==================================================
    # ARRÊT PROPRE
    # ==================================================
    def stop(self):
        self.running = False
        self._connected_event.clear()

        if self.ser:
            try:
                self.ser.close()
            except:
                pass
            self.ser = None

        if self.on_status:
            self.on_status(False)
