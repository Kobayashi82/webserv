<?php
session_start();

// Destruir la sesión y eliminar todas las variables de sesión
session_unset(); // Elimina todas las variables de sesión
session_destroy(); // Destruye la sesión

// Eliminar las cookies de sesión si están configuradas
setcookie('user_session_cookie', '', time() - 3600, "/"); // Eliminar la cookie de sesión

// Redirigir al login
header('Location: /login.php');
exit();
