<?php
session_start();

// Comprobamos si se ha enviado el formulario de login
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = $_POST['username'];
    $password = $_POST['password'];

    // Simulación de una verificación en base de datos
    // En un caso real, se haría una consulta a la base de datos
    $valid_user = 'admin'; // Usuario en la base de datos
    $valid_pass = '1234';  // Contraseña en la base de datos

    // Verificar que el usuario y contraseña sean correctos
    if ($username === $valid_user && $password === $valid_pass) {
        // Las credenciales son correctas, creamos una cookie de sesión
        $session_id = session_id();
        setcookie('user_session', $session_id, time() + 3600, "/"); // Expira en 1 hora

        // Redirigir al home
        header('Location: home.php');
        exit();
    } else {
        // Credenciales incorrectas, mensaje de error
        $error_message = "Nombre de usuario o contraseña incorrectos";
    }
}
?>

<!-- Formulario de Login HTML -->
<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
</head>
<body>
    <h2>Iniciar Sesión</h2>

    <?php
    // Mostrar el mensaje de error si existe
    if (isset($error_message)) {
        echo "<p style='color: red;'>$error_message</p>";
    }
    ?>

    <form method="POST" action="login.php">
        <label for="username">Usuario:</label>
        <input type="text" name="username" id="username" required><br>

        <label for="password">Contraseña:</label>
        <input type="password" name="password" id="password" required><br>

        <input type="submit" value="Iniciar Sesión">
    </form>
</body>
</html>
