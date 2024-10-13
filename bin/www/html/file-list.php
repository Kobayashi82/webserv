<?php
session_start();

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');
    exit();
}

// Obtiene el nombre del usuario en minúsculas
$username = strtolower($_SESSION['user_session']);
$userDirectory = 'users/' . $username;

// Verifica si la carpeta del usuario existe
if (!is_dir($userDirectory)) {
    echo json_encode(['status' => 'error', 'message' => 'No se encontró la carpeta del usuario.']);
    exit();
}

// Lee los archivos dentro de la carpeta del usuario
$files = scandir($userDirectory);
$filesList = [];
foreach ($files as $file) {
    // Ignora las carpetas '.' y '..'
    if ($file == '.' || $file == '..') {
        continue;
    }

    // Verifica si es un archivo (no una carpeta)
    if (is_file($userDirectory . '/' . $file)) {
        // Obtiene el tamaño del archivo en kilobytes (KB)
        $fileSize = number_format(filesize($userDirectory . '/' . $file) / 1024, 2) . ' KB';

        // Agrega la información del archivo a la lista
        $filesList[] = ['name' => $file, 'size' => $fileSize];
    }
}

// Devuelve la lista de archivos en formato JSON
echo json_encode(['status' => 'success', 'files' => $filesList]);
?>
