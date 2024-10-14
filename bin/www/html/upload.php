<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
    exit();
}

$email = strtolower($_SESSION['user_session']);																//	Obtiene el 'email' del usuario y lo convierte a minúsculas
$userDirectory = 'users/' . $email;																			//	Define el directorio del usuario
createUserDirectory($userDirectory);																		//	Crea el directorio si no existe

if ($_SERVER['REQUEST_METHOD'] === 'POST') {																//	Procesar datos del formulario solo si se envían por POST
    $contentType = isset($_SERVER["CONTENT_TYPE"]) ? $_SERVER["CONTENT_TYPE"] : '';							//	Obtener el tipo de contenido de la solicitud

    if (strpos($contentType, 'multipart/form-data') !== false) {											//	Verificar si el tipo de contenido es 'multipart/form-data', necesario para la carga de archivos
        $boundary = substr($contentType, strpos($contentType, "boundary=") + 9);							//	Obtener el boundary del Content-Type (separador entre partes del formulario)
        $boundary = "--" . $boundary;																		//	Agregar el prefijo '--' al boundary

        
        $inputStream = fopen('php://input', 'rb');															//	Abrir el flujo de entrada para leer los datos del archivo
        $fileHandle = null;																					//	Inicializar el manejador de archivo como nulo
        $isFileData = false;																				//	Variable de control para saber si estamos procesando datos de un archivo

        while (!feof($inputStream)) {																		//	Leer línea por línea hasta el final del flujo de entrada
            $line = fgets($inputStream);																	//	Leer una línea del flujo de entrada

            if (strpos($line, $boundary) !== false) {														//	Detectar si estamos en un nuevo boundary (inicio de una nueva parte)
                if ($isFileData && $fileHandle) {															//	Si estábamos escribiendo un archivo y encontramos el boundary
                    fclose($fileHandle);																	//	Cerramos el archivo si estábamos escribiendo en uno
                    $fileHandle = null;																		//	Restablecer el manejador del archivo
                 }
                $isFileData = false;																		//	Cambiar el estado, ya no estamos procesando datos del archivo
            }

            if (preg_match('/filename="(.+)"/', $line, $matches)) {											//	Si encontramos la línea que contiene el nombre del archivo
                $fileName = basename($matches[1]);															//	Asegurarse de que solo obtenemos el nombre del archivo sin rutas
                $filePath = $userDirectory . '/' . $fileName;												//	Definir la ruta del archivo en el servidor

                $fileHandle = fopen($filePath, 'wb');														//	Abrir el archivo de destino para escribir los datos del archivo
                $isFileData = true;																			//	Establecer el estado para indicar que estamos procesando datos del archivo
            }

            if ($isFileData && strpos($line, "\r\n") === 0) {												//	Si encontramos la línea vacía que separa los encabezados del contenido del archivo
                while (!feof($inputStream)) {																//	Leer el contenido real del archivo y escribirlo en el archivo de destino
                    $chunk = fgets($inputStream);															//	Leer un trozo de datos del archivo
                    if (strpos($chunk, $boundary) !== false) break;											//	Detener la lectura cuando encontramos el siguiente boundary (fin de la carga del archivo)
                    fwrite($fileHandle, $chunk);															//	Escribir los datos leídos en el archivo
                }
            }
        }

        fclose($inputStream);																				//	Cerrar el flujo de entrada cuando hayamos terminado de procesar todos los datos
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Formato no soportado']);						//	Si el tipo de contenido no es 'multipart/form-data', enviar un mensaje de error
    }
} else {
    echo json_encode(['status' => 'error', 'message' => 'Metodo no permitido']);							//	Si no es un método POST, enviar un mensaje de error
}
?>