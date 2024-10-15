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

if (!$filePath || strncmp($filePath, realpath($userDirectory), strlen(realpath($userDirectory))) !== 0) {	//	Comprobar que el archivo está dentro del directorio permitido
    echo json_encode(['status' => 'error', 'message' => 'Ruta no valida']);									//	Si la ruta no es válida, devolvemos un mensaje de "failed" al cliente
	exit();
}

if (is_file($filePath)) {																					//	Verifica si el archivo existe y es un archivo válido
    if (unlink($filePath)) {																				//	Eliminar el archivo
        echo json_encode(['status' => 'success', 'message' => 'Archivo eliminado correctamente']);			//	Enviar un mesaje de "success" al cliente
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Error al eliminar el archivo']);				//	Si falla, devolvemos un mensaje de "failed" al cliente
    }
} else {
    echo json_encode(['status' => 'error', 'message' => 'Archivo no encontrado']);							//	Si no existe, devolvemos un mensaje de "failed" al cliente
}

exit();
?>
