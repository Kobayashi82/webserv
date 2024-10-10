#!/usr/bin/python3

import sys

def main():
    # Leer el cuerpo del mensaje desde stdin
    body_content = sys.stdin.read()

    # Imprimir el cuerpo del mensaje recibido
    if body_content:
        print("Cuerpo recibido:")
        print(body_content)
    else:
        print("No se recibió ningún dato en el cuerpo.")

if __name__ == "__main__":
    main()
