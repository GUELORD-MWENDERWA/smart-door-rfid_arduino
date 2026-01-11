from core.serial_link import SerialLink
from prompt_toolkit import PromptSession
import json

def main():
    link = SerialLink()
    ports = link.list_ports()
    print("Ports disponibles :", ports)
    port = input("Port : ")
    link.connect(port)

    session = PromptSession()
    while True:
        cmd = session.prompt("> ")
        if cmd == "exit":
            break
        try:
            link.send(json.loads(cmd))
        except:
            print("JSON invalide")

if __name__ == "__main__":
    main()
