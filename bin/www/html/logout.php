<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

session_unset();																							//	Eliminar todas las variables de sesión
session_destroy();																							//	Destruir la sesión

setcookie('user_session_cookie', '', time() - 3600, "/");													//	Eliminar la cookie de sesión

header('Location: /login.php');																				//	Redirigir al login
?>