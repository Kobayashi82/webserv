<?php
session_start();

// Función para verificar si las credenciales almacenadas en la cookie son válidas
function checkUserSession($username) {
    $userdata = @file_get_contents('userdata');
    if ($userdata === false) {
        return false;
    }

    $lines = explode("\n", $userdata);
    foreach ($lines as $line) {
        $line = trim($line);
        if ($line === '') {
            continue;
        }
        list($storedUser, $storedPass) = explode(';', $line);

        if ($storedUser == $username) {
            return true;
        }
    }
    return false;
}

if (isset($_COOKIE['user_session_cookie'])) {
    $username = $_COOKIE['user_session_cookie'];

    if (checkUserSession($username)) {
        $_SESSION['user_session'] = $username;
    } else {
        setcookie('user_session_cookie', '', time() - 3600, "/");
        unset($_COOKIE['user_session_cookie']);
    }
}

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');
    exit();
}

$username = strtolower($_SESSION['user_session']);
$userDirectory = __DIR__ . '/users/' . $username;

if (!is_dir($userDirectory)) {
    if (!mkdir($userDirectory, 0777, true)) {
        echo json_encode(['status' => 'error', 'message' => 'No se pudo crear el directorio.']);
        exit();
    }
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $contentType = isset($_SERVER["CONTENT_TYPE"]) ? $_SERVER["CONTENT_TYPE"] : '';

    if (strpos($contentType, 'multipart/form-data') !== false) {
        // Obtener el boundary del Content-Type
        $boundary = substr($contentType, strpos($contentType, "boundary=") + 9);
        $boundary = "--" . $boundary;

        // Abrir el flujo de entrada para leer los datos del archivo
        $inputStream = fopen('php://input', 'rb');
        $fileHandle = null;
        $isFileData = false;

        while (!feof($inputStream)) {
            $line = fgets($inputStream);

            // Detectar si estamos en un nuevo boundary
            if (strpos($line, $boundary) !== false) {
                if ($isFileData && $fileHandle) {
                    // Cerramos el archivo si estábamos escribiendo en uno
                    fclose($fileHandle);
                    $fileHandle = null;
                    echo "Archivo subido correctamente.<br>";
                }
                $isFileData = false;
            }

            // Si encontramos el nombre del archivo
            if (preg_match('/filename="(.+)"/', $line, $matches)) {
                $fileName = basename($matches[1]); // Asegurarse de que sea solo el nombre del archivo
                $filePath = $userDirectory . '/' . $fileName;

                // Abrir el archivo de destino para escritura
                $fileHandle = fopen($filePath, 'wb');
                $isFileData = true;
            }

            // Si encontramos la línea vacía que separa los encabezados del contenido
            if ($isFileData && strpos($line, "\r\n") === 0) {
                // Leer el contenido real del archivo y escribirlo
                while (!feof($inputStream)) {
                    $chunk = fgets($inputStream);
                    if (strpos($chunk, $boundary) !== false) {
                        // Detenemos la escritura cuando encontramos el siguiente boundary
                        break;
                    }
                    fwrite($fileHandle, $chunk); // Escribir en el archivo
                }
            }
        }

        // Cerrar el flujo de entrada
        fclose($inputStream);
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Formato no soportado.']);
    }
} else {
    echo json_encode(['status' => 'error', 'message' => 'Método no permitido.']);
}
