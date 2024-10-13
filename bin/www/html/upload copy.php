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

// Directorio donde se almacenarán los archivos subidos
$userDirectory = __DIR__ . '/users/' . $username;  // Usar __DIR__ para asegurar la ruta completa

// Verifica si el directorio existe, si no, lo crea
if (!is_dir($userDirectory)) {
    if (!mkdir($userDirectory, 0777, true)) {
        echo json_encode(['status' => 'error', 'message' => 'No se pudo crear el directorio.']);
        exit();
    }
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {

    
    // Leer el contenido de la solicitud directamente desde stdin
    $inputData = file_get_contents('php://input');

    // Procesa el contenido usando php://input en lugar de $_FILES
    $multipartData = parseMultipartData($inputData);

    // Simula el arreglo $_FILES con los datos procesados
    $_FILES = simulateFilesArray($multipartData);

    // Ahora $_FILES estará disponible para su uso como en una solicitud normal
    // echo '<pre>';
    // print_r($_FILES);
    // echo '</pre>';

    // Abre el archivo en modo binario de solo lectura
    ini_set('memory_limit', '1024M');  // Aumenta a 1024 MB

    foreach ($_FILES as $fileField => $fileData) {
        if (!empty($fileData['content'])) {
            $fileName = basename($fileData['name']);  // Evitar nombres de archivo peligrosos
            $fileContent = $fileData['content'];
            $targetFilePath = $userDirectory . '/' . $fileName;  // Ruta completa al directorio 'users' y subcarpeta del usuario

            // Depuración: Verifica la ruta donde se guardará el archivo
            echo "Ruta de destino para el archivo: " . $targetFilePath . "<br>";

            // Guardar el archivo en el servidor
            if (file_put_contents($targetFilePath, $fileContent)) {
                echo "Archivo '$fileName' subido correctamente.<br>";
            } else {
                echo "Error al subir el archivo '$fileName'.<br>";
            }
        }
    }
} else {
    // Si no es una solicitud POST, se redirige al login o página principal
    echo json_encode(['status' => 'error', 'message' => 'Método no permitido.']);
}

/**
 * Función para procesar los datos de multipart/form-data
 */
function parseMultipartData($input) {
    $contentType = $_SERVER['CONTENT_TYPE'];
    preg_match('/boundary=(.*)$/', $contentType, $matches);
    if (!isset($matches[1])) {
        throw new Exception("No se pudo encontrar el boundary en el encabezado Content-Type.");
    }
    $boundary = $matches[1];

    $parts = array();
    $sections = explode("--" . $boundary, $input);

    foreach ($sections as $section) {
        if (empty($section) || $section == "--") {
            continue;
        }

        if (preg_match('/Content-Disposition: form-data; name="(.+)"; filename="(.+)"/', $section, $matches)) {
            $name = $matches[1];
            $filename = $matches[2];
            $content = substr($section, strpos($section, "\r\n\r\n") + 4);
            $parts[$name] = array(
                'filename' => $filename,
                'content' => $content
            );
        } elseif (preg_match('/Content-Disposition: form-data; name="(.+)"/', $section, $matches)) {
            $name = $matches[1];
            $content = substr($section, strpos($section, "\r\n\r\n") + 4);
            $parts[$name] = $content;
        }
    }

    return $parts;
}

function simulateFilesArray($multipartData) {
    $files = array();
    $fileIndex = 0;  // Inicia el índice para manejar múltiples archivos

    foreach ($multipartData as $name => $data) {
        if (isset($data['filename'])) {
            $tmpFilePath = tempnam(sys_get_temp_dir(), 'upload_');  // Crea un archivo temporal
            file_put_contents($tmpFilePath, $data['content']);  // Escribe el contenido en el archivo temporal

            // Simula el arreglo $_FILES
            $files['file' . $fileIndex] = array(
                'name' => $data['filename'],
                'type' => mime_content_type($tmpFilePath),  // Usa la ruta temporal para obtener el tipo MIME
                'tmp_name' => $tmpFilePath,  // El archivo temporal
                'error' => UPLOAD_ERR_OK,  // No hubo error al subir
                'size' => strlen($data['content']),  // Tamaño del archivo
                'content' => $data['content']  // El contenido del archivo
            );
            $fileIndex++;  // Incrementa el índice para el siguiente archivo
        }
    }

    return $files;
}
