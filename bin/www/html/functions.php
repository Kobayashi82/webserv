<?php

function checkUserSession($email, $pass) {
	$userdata = @file_get_contents('users/userdata');																//	Abrir el archivo de usuarios
    if ($userdata === false) return false;

    $lines = explode("\n", $userdata);																				//	Procesar las líneas del archivo de usuarios
    foreach ($lines as $line) {
        $line = trim($line);																						//	Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue;																					//	Ignorar líneas vacías
		$parts = explode(';', $line);
		if (count($parts) < 4) continue;																			// Si no tiene las 4 partes, continuar con la siguiente línea

        list($storedUser, $storedPass, $storedFirstName, $storedLastName) = explode(';', $line);					//	Dividir la cadena en 'email', 'pass', 'firstname', y 'lastname'
        if (strtolower($storedUser) == strtolower($email) && $storedPass == $pass) return true;						//	Comprobar si el 'email' y 'pass' coincide
    }

    return false;
}

function UserSession() {
	
	$userdata = @file_get_contents('users/userdata');																//	Abrir el archivo de usuarios
    if ($userdata === false) {
		session_unset();																							//	Eliminar todas las variables de sesión
		session_destroy();																							//	Destruir la sesión
		setcookie('user_session_cookie', '', time() - 3600, "/", "", false, true);										//	Eliminar la cookie de sesión
		unset($_COOKIE['user_session_cookie']);																		//	Eliminar la cookie del entorno
		session_start();
		return false;
	}

	if (isset($_SESSION['user_session'])) {
		$s_email = strtolower($_SESSION['user_session']['email']);
		$s_pass = $_SESSION['user_session']['pass'];
		if (!checkUserSession($s_email, $s_pass)) {
			session_unset();																						//	Eliminar todas las variables de sesión
			session_destroy();																						//	Destruir la sesión
			setcookie('user_session_cookie', '', time() - 3600, "/", "", false, true);									//	Eliminar la cookie de sesión
			unset($_COOKIE['user_session_cookie']);																	//	Eliminar la cookie del entorno
			session_start();
			return false;
		}
	}

	if (isset($_COOKIE['user_session_cookie'])) {																	//	Verificar si la cookie de sesión existe y tiene un valor
		list($email, $pass) = explode(';', base64_decode($_COOKIE['user_session_cookie']));  						//	Separar 'email', 'pass' de la cookie

		if (checkUserSession($email, $pass)) {																		//	Verificar si el nombre de 'email' y la 'pass'' son válidos
			$_SESSION['user_session'] = array('email' => $email, 'pass' => $pass);									//	Iniciar sesión automáticamente si la cookie es válida
		} else {
			setcookie('user_session_cookie', '', time() - 3600, "/", "", false, true);									//	Eliminar la cookie del cliente
			unset($_COOKIE['user_session_cookie']);																	//	Eliminar la cookie del entorno
		}
	}
}

function createUserDirectory($userDirectory) {																		//	Verifica si la carpeta existe, si no, crea la carpeta
	if (!is_dir($userDirectory) && !mkdir($userDirectory, 0777, true)) {
		echo json_encode(['status' => 'error', 'message' => 'No se pudo crear el directorio ' . $userDirectory]);
		exit();
	}
}

function deleteDirectory($dir) {
    if (!is_dir($dir)) return false;																				//	Si no es un directorio, retorna false

    $files = array_diff(scandir($dir), array('.', '..'));															//	Obtiene todos los archivos y carpetas dentro del directorio exceptop "." y ".."

    foreach ($files as $file) {
        $filePath = $dir . DIRECTORY_SEPARATOR . $file;
        if (is_dir($filePath)) {
            deleteDirectory($filePath);																				//	Si es un subdirectorio, se llama recursivamente a deleteDirectory
        } else {
            unlink($filePath);																						//	Si es un archivo, se elimina con unlink
        }
    }

    return rmdir($dir);																								//	Elimina el directorio
}

function formatFileSize($size) {																					//	Función para formatear el tamaño del archivo en KB, MB, GB, etc.
    $unit = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB'];
    $mod = 1024;
    $i = 0;

    while ($size >= $mod && $i < count($unit) - 1) {
        $size /= $mod;
        $i++;
    }

    return round(floatval($size), 2) . ' ' . $unit[$i];																//	Usamos floatval() para asegurar que el valor de $size es un número
}

?>