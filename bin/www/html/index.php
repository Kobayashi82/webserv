<?php
session_start();

// Función para verificar si las credenciales almacenadas en la cookie son válidas
function checkUserSession($username) {
    // Intentar abrir el archivo de usuarios
    $userdata = @file_get_contents('userdata');
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
} else {
    // Si no existe la cookie, redirigir al login
    header('Location: /login.php');
    exit();
}
?>
