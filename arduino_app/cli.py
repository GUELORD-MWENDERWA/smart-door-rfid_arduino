from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter
from serial_link import SerialLink
import json


COMMANDS = [
    # --- Auth admin ---
    '{"cmd":"123"}',

    # --- Commandes admin ---
    '{"cmd":"11"}',        # Ajouter badge
    '{"cmd":"12"}',        # Supprimer badge
    '{"cmd":"13"}',        # Lister badges
    '{"cmd":"14"}',        # Reset EEPROM (demande confirmation)

    # --- Reset confirmation ---
    '{"cmd":"99"}',        # Confirmer reset
    '{"cmd":"00"}',        # Annuler reset

    # --- Changer PIN ---
    '{"cmd":"99123"}',     # Exemple : nouveau PIN = 123

    # --- Ouvrir porte ---
    '{"cmd":"10"}',      # Ouvrir porte seulement admin validé

    # --- Quitter ---
    'exit'
]


def main():
    link = SerialLink(port="COM3", baudrate=115200)
    link.start()

    completer = WordCompleter(
        COMMANDS,
        ignore_case=True
    )

    session = PromptSession(completer=completer)

    try:
        while True:
            text = session.prompt("> ").strip()

            if not text:
                continue

            if text.lower() == "exit":
                break

            try:
                payload = json.loads(text)
            except json.JSONDecodeError:
                print("[ERROR] JSON invalide")
                continue

            link.send(payload)

    except KeyboardInterrupt:
        pass
    finally:
        link.stop()


if __name__ == "__main__":
    main()
