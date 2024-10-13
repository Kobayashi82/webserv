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

// Verifica si el usuario está logueado (por sesión o cookie)
if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');
    exit();
}

// Obtiene el nombre de usuario y convierte a minúsculas
$username = strtolower($_SESSION['user_session']);

// Define el directorio del usuario en minúsculas
$userDirectory = 'users/' . $username;

// Obtiene el archivo a eliminar desde la URL
$file = $_GET['file'] ?? '';
$filePath = $userDirectory . '/' . $file;

// Verifica si el archivo existe y es un archivo válido
if (is_file($filePath)) {
    // Eliminar el archivo
    if (unlink($filePath)) {
        echo json_encode(['status' => 'success', 'message' => 'Archivo eliminado correctamente']);
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Error al eliminar el archivo']);
    }
} else {
    echo json_encode(['status' => 'error', 'message' => 'Archivo no encontrado']);
}

exit();
