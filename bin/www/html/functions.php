<?php

function checkUserSession($email, $pass) {
    $userdata = @file_get_contents('users/userdata');														//	Abrir el archivo de usuarios
    if ($userdata === false) return false;

    $lines = explode("\n", $userdata);																		//	Procesar las líneas del archivo de usuarios
    foreach ($lines as $line) {
        $line = trim($line);																				//	Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue;																			//	Ignorar líneas vacías

        list($stored_email, $stored_pass) = explode(';', $line);											//	Dividir la cadena en 'email' y 'pass'
        if ($stored_email == $email && $stored_pass == $pass) return true;									//	Comprobar si el 'email' y 'pass' coincide
    }

    return false;
}

function UserSession() {
	if (isset($_COOKIE['user_session_cookie'])) {															//	Verificar si la cookie de sesión existe y tiene un valor
		list($email, $pass) = explode(';', base64_decode($_COOKIE['user_session_cookie']));					//	Separar 'email' y 'pass' de la cookie

		if (checkUserSession($email, $pass)) {																//	Verificar si el nombre de 'email' y la 'pass'' son válidos
			$_SESSION['user_session'] = $email;																//	Iniciar sesión automáticamente si la cookie es válida
		} else {
			setcookie('user_session_cookie', '', time() - 3600, "/");										//	Eliminar la cookie del cliente
			unset($_COOKIE['user_session_cookie']);															//	Eliminar la cookie del entorno
		}
	}
}

function createUserDirectory($userDirectory) {																//	Verifica si la carpeta existe, si no, crea la carpeta
	if (!is_dir($userDirectory) && !mkdir($userDirectory, 0777, true)) {
		echo json_encode(['status' => 'error', 'message' => 'No se pudo crear el directorio ' . $userDirectory]);
		exit();
	}
}

function formatFileSize($size) {																			//	Función para formatear el tamaño del archivo en KB, MB, GB, etc.
    $unit = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB'];
    $mod = 1024;
    $i = 0;

    while ($size >= $mod && $i < count($unit) - 1) {
        $size /= $mod;
        $i++;
    }

    return round(floatval($size), 2) . ' ' . $unit[$i];														//	Usamos floatval() para asegurar que el valor de $size es un número
}

?>