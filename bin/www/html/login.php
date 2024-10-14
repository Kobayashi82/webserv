<?php
session_start();

// Función para verificar si las credenciales almacenadas en la cookie son válidas
function checkUserSession($username) {
    
    // Intentar abrir el archivo de usuarios
    $userdata = @file_get_contents('users/userdata');
    if ($userdata === false) {
        return false;
    }

    // Procesar las líneas del archivo de usuarios
    $lines = explode("\n", $userdata);
    foreach ($lines as $line) {
        $line = trim($line); // Eliminar espacios y saltos de línea innecesarios
        if ($line === '') {
            continue; // Ignorar líneas vacías
        }

        list($storedUser, $storedPass) = explode(';', $line);

        // Comprobar si el usuario coincide
        if ($storedUser == $username) {
            return true;
        }
    }
    return false;
}

// Verificar si la cookie de sesión existe y tiene un valor
if (isset($_COOKIE['user_session_cookie'])) {
    $username = $_COOKIE['user_session_cookie'];

    // Verificar si el nombre de usuario en la cookie es válido
    if (checkUserSession($username)) {
        // Iniciar sesión automáticamente si la cookie es válida
        $_SESSION['user_session'] = $username;
    } else {
        // Si la cookie no es válida, eliminarla
        setcookie('user_session_cookie', '', time() - 3600, "/");
        unset($_COOKIE['user_session_cookie']);
    }
}

// Si ya está logueado, redirigir
if (isset($_SESSION['user_session'])) {
    header('Location: /home.php');
    exit();
}

// Procesar datos del formulario solo si se envían por POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Leer los datos del cuerpo de la solicitud (stdin)
    parse_str(file_get_contents("php://input"), $postData);

    // Verificar si los datos están presentes
    $username = isset($postData['u']) ? $postData['u'] : '';
    $password = isset($postData['p']) ? $postData['p'] : '';

    // Verificar si el archivo userdata existe
    $userdata = @file_get_contents('users/userdata');
    if ($userdata === false) {
        echo json_encode(['success' => false, 'message' => 'User data file not found']);
        exit();
    }

    // Procesar las líneas del archivo de usuarios
    $lines = explode("\n", $userdata);
    foreach ($lines as $line) {
        $line = trim($line); // Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue; // Ignorar líneas vacías

        list($storedUser, $storedPass) = explode(';', $line);

        // Comprobar si el usuario y la contraseña coinciden
        if (strtolower($storedUser) == strtolower($username) && $storedPass == $password) {

            // Crear la sesión y la cookie para recordar al usuario
            $_SESSION['user_session'] = $username;

            // Verificar si el usuario ha marcado la opción "Recordarme"
            if (isset($postData['remember_me']) && $postData['remember_me'] == '1') {
                // Crear cookie para recordar usuario por 7 días
                setcookie('user_session_cookie', $username, time() + (7 * 24 * 60 * 60), "/"); // 7 días
            } else {
                // Si no se ha marcado, eliminar la cookie si existe
                if (isset($_COOKIE['user_session_cookie'])) {
                    setcookie('user_session_cookie', '', time() - 3600, "/");
                }
            }

            echo json_encode(['success' => true, 'message' => 'Login successful']);
            exit();
        }
    }

    // Si no coincide ningún usuario, devolver un mensaje de error
    echo json_encode(['success' => false, 'message' => 'Invalid username or password']);
    exit();
}
?>


<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login</title>
    <link rel="stylesheet" href="style.css">
</head>
<body>
<div class="login">
    <h1>Login</h1>
    <form id="loginForm" novalidate> <!-- Añadido novalidate para desactivar la validación nativa -->
        <input type="email" name="u" id="username" placeholder="Email" required="required" />
        <input type="password" name="p" id="password" placeholder="Password" required="required" />
        
        <label class="remember-me">
            <input type="checkbox" name="remember_me" value="1"> Remember me
        </label>

        <button type="submit" class="btn btn-primary btn-block btn-large">Continue</button>
    </form>

    <p style="text-align: center; margin-top: 15px; font-size: 12px; color: gray;">
        Don't have an account? <a href="/signup.php" style="color: yellow; margin-left: 5px;">Sign Up</a>
    </p>
    <p id="error-message"></p>
</div>

<script>
document.getElementById('loginForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    // Validaciones personalizadas
    const username = document.getElementById('username');
    const password = document.getElementById('password');
    
    let formIsValid = true; // Variable para saber si el formulario es válido

    // Validación del campo de usuario (email)
    const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    if (username.value === '') {
        username.setCustomValidity('Please enter your email');
        formIsValid = false;
    } else if (!emailRegex.test(username.value)) { // Comprobamos si el email es válido con expresion regular
        username.setCustomValidity('Please enter a valid email');
        formIsValid = false;
    } else {
        username.setCustomValidity('');
    }

    // Validación del campo de contraseña
    if (password.value === '') {
        password.setCustomValidity('Please enter your password');
        formIsValid = false;
    } else {
        password.setCustomValidity('');
    }

    // Si algún campo no es válido, no enviamos el formulario
    if (!formIsValid) {
        document.getElementById('loginForm').reportValidity();
        return; // Salir del submit
    }

    // Si la validación es correcta, enviamos el formulario usando Fetch API
    const rememberMe = document.querySelector('input[name="remember_me"]').checked ? '1' : '0';

    fetch('/login.php', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `u=${encodeURIComponent(username.value)}&p=${encodeURIComponent(password.value)}&remember_me=${rememberMe}`
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            window.location.href = '/home.php'; // Redirige a home.php en caso de éxito
        } else {
            const errorMessage = document.getElementById('error-message');
            errorMessage.textContent = data.message; // Muestra el mensaje de error justo debajo del botón
        }
    })
    .catch(error => {
        console.error('Error:', error);
        alert('There was an error processing your request. Please try again later');
    });
});
</script>
</body>
</html>
