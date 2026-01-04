import serial
import time
import json

SERIAL_PORT = 'COM3'
BAUD_RATE = 9600

with serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=10) as ser:
    time.sleep(2)

    cmd = {"cmd": "123#"}
    cmd_str = json.dumps(cmd) + '\n'
    ser.write(cmd_str.encode('utf-8'))
    print(f"Commande envoyée: {cmd_str.strip()}")

    start_time = time.time()
    while time.time() - start_time < 10:
        if ser.in_waiting:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print("Arduino:", line)
        time.sleep(0.1)

input("Appuyez sur Entrée pour fermer le script...")
