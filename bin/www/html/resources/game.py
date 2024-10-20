#!/usr/bin/env python3
import cgi
import random
import os

# Obtener datos del formulario
form = cgi.FieldStorage()
user_choice = form.getvalue('choice')

# Opciones
choices = ['piedra', 'papel', 'tijeras']
computer_choice = random.choice(choices)

# Función para determinar el ganador
def determinar_ganador(user, computer):
    if user == computer:
        return "¡Empate!"
    elif (user == 'piedra' and computer == 'tijeras') or \
         (user == 'papel' and computer == 'piedra') or \
         (user == 'tijeras' and computer == 'papel'):
        return "¡Ganaste!"
    else:
        return "Perdiste..."

# Comienza a imprimir el HTML
print("Content-type: text/html\r\n\r\n")
print("""
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Piedra, Papel o Tijeras</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            padding-top: 50px;
            background-color: #3a3a4b;
            color: #d1d1d1;
        }
        h1 {
            text-align: center;
            color: #85c48e;
            font-size: 2.5rem;
            margin-bottom: 100px;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
        }
        .choice {
            display: inline-block;
            margin: 0 15px;
            cursor: pointer;
        }
        button {
            background: none;
            border: none;
            padding: 0;
            cursor: pointer;
        }
        button img {
            width: 150px;
            height: 150px;
            display: block;
            margin: 0 auto;
        }
        button:hover img {
            opacity: 0.9;
        }
        .result-container {
            margin-top: 100px;
        }
        .result {
            font-size: 1.5rem;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <h1>Piedra, Papel o Tijeras</h1>
    <form method="post" action="">
        <div class="choice">
            <button type="submit" name="choice" value="piedra">
                <img src="/resources/rock.png" alt="Piedra">
            </button>
        </div>
        <div class="choice">
            <button type="submit" name="choice" value="papel">
                <img src="/resources/paper.png" alt="Papel">
            </button>
        </div>
        <div class="choice">
            <button type="submit" name="choice" value="tijeras">
                <img src="/resources/scissor.png" alt="Tijeras">
            </button>
        </div>
    </form>

    <!-- Contenedor para el resultado -->
    <div class="result-container">
""")

# Mostrar el resultado solo si el usuario ha hecho una elección
if user_choice:
    resultado = determinar_ganador(user_choice, computer_choice)
    print(f"<p>Elegiste: {user_choice}</p>")
    print(f"<p>La computadora eligió: {computer_choice}</p>")
    print(f"<p class='result'>{resultado}</p>")

print("""
    </div>
	<p>Ruta actual: {}</p>
</body>
</html>
""".format(os.getcwd()))
