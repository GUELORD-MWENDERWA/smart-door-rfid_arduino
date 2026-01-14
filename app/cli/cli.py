from core.serial_link import SerialLink
from prompt_toolkit import PromptSession
import json
import sys


def main():
    link = SerialLink()

    # ============================
    # Sélection du port
    # ============================
    ports = link.list_ports()
    if not ports:
        print("Aucun port série détecté")
        return

    print("Ports disponibles :")
    for p in ports:
        print(" -", p)

    port = input("Port : ").strip()
    if not port:
        print("Port invalide")
        return

    # ============================
    # Connexion
    # ============================
    link.connect(port)

    print("Connexion en cours...")
    if not link.wait_connected(5):
        print("Échec connexion Arduino")
        return

    print("Connecté ✔")
    print("Entrer une commande JSON (ex: {\"cmd\":\"123\"}) ou 'exit'")

    # ============================
    # Boucle CLI
    # ============================
    session = PromptSession()

    try:
        while True:
            raw = session.prompt("> ").strip()

            if not raw:
                continue

            if raw.lower() in ("exit", "quit"):
                break

            try:
                payload = json.loads(raw)
            except json.JSONDecodeError:
                print("❌ JSON invalide")
                continue

            try:
                link.send(payload)
            except RuntimeError as e:
                print("❌ Erreur série :", e)
            except Exception as e:
                print("❌ Erreur inconnue :", e)

    except KeyboardInterrupt:
        print("\nArrêt CLI")

    finally:
        link.stop()
        print("Déconnecté")


if __name__ == "__main__":
    main()
