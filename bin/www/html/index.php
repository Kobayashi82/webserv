<?php
ini_set('session.cookie_httponly', 1); 

session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (isset($_SESSION['user_session'])) {																		//	Si ya está logueado, redirigir a 'home.php'
    header('Location: /home.php');
} else {
    header('Location: /login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
}
?>
