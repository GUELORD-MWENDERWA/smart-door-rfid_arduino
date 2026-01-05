import serial
import threading
import json
import time


class SerialLink:
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = None
        self.running = False
        self.thread = None

    def start(self):
        self.ser = serial.Serial(
            self.port,
            self.baudrate,
            timeout=0.1,
            write_timeout=0.1
        )

        # Reset Arduino
        time.sleep(2)

        self.running = True
        self.thread = threading.Thread(
            target=self._reader,
            daemon=True
        )
        self.thread.start()

        print(f"[INFO] Port {self.port} ouvert à {self.baudrate} bauds")

    def _reader(self):
        while self.running:
            try:
                if self.ser.in_waiting:
                    line = self.ser.readline().decode("utf-8", errors="ignore").strip()
                    if not line:
                        continue

                    try:
                        obj = json.loads(line)
                        print("[RX]", obj)
                    except json.JSONDecodeError:
                        print("[RX][RAW]", line)

            except serial.SerialException as e:
                print("[ERROR] Série:", e)
                self.running = False

    def send(self, payload: dict):
        if not self.ser or not self.ser.is_open:
            print("[ERROR] Port série non ouvert")
            return

        data = json.dumps(payload) + "\n"
        self.ser.write(data.encode("utf-8"))
        self.ser.flush()

        print("[TX]", payload)

    def stop(self):
        self.running = False

        if self.thread:
            self.thread.join(timeout=1)

        if self.ser and self.ser.is_open:
            self.ser.close()

        print("[INFO] Port série fermé")
