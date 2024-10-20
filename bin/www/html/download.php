<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
    exit();
}

$email = strtolower($_SESSION['user_session']['email']);													//	Obtiene el 'email' del usuario y lo convierte a minúsculas
$userDirectory = 'users/' . $email;																			//	Define el directorio del usuario

$file = $_GET['file'] ?? '';																				//	Obtiene el archivo a eliminar desde la URL
$filePath = $userDirectory . '/' . $file;																	//	Obtiene la ruta completa al archivo

if (!$filePath || strncmp(realpath($filePath), realpath($userDirectory), strlen(realpath($userDirectory))) !== 0) {	//	Comprobar que el archivo está dentro del directorio permitido
	header("HTTP/1.1 404 Not Found");
    echo json_encode(['status' => 'error', 'message' => 'Archivo no encontrado']);							//	Si la ruta no es válida, devolvemos un mensaje de "failed" al cliente
	exit();
}

if (is_file($filePath)) {																					//	Verifica si el archivo existe y es un archivo válido
    header('Content-Type: application/octet-stream');														//	Establece el tipo de archivo
    header('Content-Disposition: attachment; filename="' . basename($file) . '"');							//	Indica que el archivo debe descargarse y define su nombre en la descarga
    header('Content-Length: ' . filesize($filePath));														//	Establece el tamaño del archivo
    header('Cache-Control: no-cache, no-store, must-revalidate');											//	No usar la caché del cliente

    ini_set('memory_limit', '1024M');																		//	Aumenta a 1024 MB el límite de memoria de PHP

    $file = fopen($filePath, 'rb');																			//	Abre el archivo en modo binario de solo lectura

    while (!feof($file)) {																					//	Lee todo el contenido del archivo
        echo fread($file, 8192);																			//	Lee y envía el archivo en chunks de 8 KB
        flush();																							//	Fuerza el vaciado del buffer para que se escriban los datos
    }

    fclose($file);																							//	Cierra el archivo
} else {
	header("HTTP/1.1 404 Not Found");
	echo json_encode(['status' => 'error', 'message' => 'Archivo no encontrado']);							//	Si no existe, devolvemos un mensaje de "failed" al cliente
}

header("HTTP/1.1 200 OK");
exit();
?>
