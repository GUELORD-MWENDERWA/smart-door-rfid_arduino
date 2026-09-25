# Smart Door Controller (RFID + Keypad)

An access control system for a single door, built on an Arduino Mega 2560. Users open the door with an authorized RFID badge. Administrators manage badges and settings from a 4x4 keypad or from a desktop application connected over USB serial.

The repository contains both the embedded firmware and the companion Python application.

## Features

**Firmware**

- RFID badge authentication with the MFRC522 reader
- Badge store persisted in EEPROM (up to 50 badges), with a magic number and schema version for safe upgrades
- PIN-protected administration: add, remove, list badges, factory reset with confirmation, change the admin PIN (3 to 6 digits)
- Explicit finite state machine (`FSMController`) covering idle, badge reading, command input, validation, execution and feedback states
- Timed relay drive for the door strike (5 s by default)
- Audible and visual feedback through a dedicated UI module
- JSON command protocol over serial (ArduinoJson)
- Size-optimized build (`-Os`, section garbage collection, no exceptions or RTTI)

**Desktop application**

- GUI built with CustomTkinter: dashboard, admin panel, badge enrollment screen, live log panel and status bar
- Interactive CLI with `prompt_toolkit`
- Serial port discovery and connection management with `pyserial`
- Protocol definitions isolated in `app/core/protocol.py`

## Architecture

```
src/
  main.cpp                  Wiring and main loop
  config.h                  Build-time defaults (admin PIN, relay timing, debug)
  fsm/FSMController         System state machine
  rfid/RFIDModule           MFRC522 wrapper
  keypad/KeypadModule       4x4 keypad input and admin PIN checks
  relay/RelayController     Timed door strike control
  eeprom/EEPROMStore        Persistent badges and admin PIN
  comm/JsonComm             JSON serial protocol
  ui/UIFeedback             Buzzer and LED feedback
app/
  main.py                   GUI entry point
  cli/cli.py                Command-line client
  core/                     Serial link, protocol, shared state
  gui/                      Views and widgets
```

## Hardware

| Component | Pins (Mega 2560) |
| --- | --- |
| MFRC522 RFID reader | SS 10, RST 9, SPI bus |
| Door strike relay | 8 |
| 4x4 matrix keypad | Rows A0-A3, columns A4, A5, 2, 3 |
| Buzzer and status LEDs | See `src/main.cpp` |

## Serial protocol

Commands are JSON objects sent at 115200 baud. Administrative commands require a valid admin PIN first.

| Command | Payload |
| --- | --- |
| Authenticate | `{"cmd": "<pin>"}` |
| Open door | `{"cmd": "10"}` |
| Add badge | `{"cmd": "11"}` |
| Remove badge | `{"cmd": "12"}` |
| List badges | `{"cmd": "13"}` |
| Request reset | `{"cmd": "14"}`, then `{"cmd": "99"}` to confirm or `{"cmd": "00"}` to cancel |
| Change PIN | `{"cmd": "99<new_pin>"}` after authentication |

## Getting started

### Firmware

```bash
pio run -t upload
pio device monitor -b 115200
```

Dependencies (`MFRC522`, `Keypad`, `ArduinoJson`) are declared in `platformio.ini`.

### Desktop application

```bash
cd app
python -m venv .venv
source .venv/bin/activate        # Windows: .venv\Scripts\activate
pip install -r requirements.txt
python main.py                   # GUI
python -m cli.cli                # CLI
```

## Security notes

- The default admin PIN is `123`. Change it on first use; the new PIN is stored in EEPROM.
- RFID UIDs can be cloned. Use this design for low-risk doors, or combine the badge with the PIN for sensitive areas.

## License

No license has been specified yet. Contact the author before reusing this code.
