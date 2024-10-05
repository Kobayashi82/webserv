<?php
session_start();

// Comprobar si la cookie de sesión existe
if (isset($_COOKIE['user_session'])) {
    // Mostrar la página de inicio
    echo "<h1>Bienvenido a la página de inicio</h1>";
    echo "<p>Usuario autenticado.</p>";
} else {
    // Si no existe la cookie, redirigir al login
    header('Location: /login.html');
    exit();
}
?>
