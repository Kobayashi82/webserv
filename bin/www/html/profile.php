<?php
session_start();

// Verificar si el usuario está logueado
if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');
    exit();
}

include('functions.php'); // Archivo con funciones comunes
$email = $_SESSION['user_session']; // Obtener el email del usuario logueado
$tempEmail = $email['email']; // Obtener solo el email

// Cargar datos del usuario actual desde el archivo
$userdataFile = 'users/userdata';
$userdata = @file_get_contents($userdataFile);
if ($userdata === false) {
    die('Error al cargar los datos del usuario.');
}

$lines = explode("\n", $userdata);
$userData = null;

foreach ($lines as $line) {
    $line = trim($line);
    if ($line === '') continue;

    $parts = explode(';', $line);
    if (count($parts) < 4) continue; // Verificar que la línea tiene al menos 4 partes

    list($storedEmail, $storedPass, $storedFirstName, $storedLastName) = $parts;

    // Verificación del email almacenado
    if (strtolower($storedEmail) === strtolower($tempEmail)) {
        $userData = [
            'email' => $storedEmail,
            'firstname' => $storedFirstName,
            'lastname' => $storedLastName,
            'password' => $storedPass
        ];
        break;
    }
}

if ($userData === null) {
    die('Usuario no encontrado.');
}

// Actualizar los datos del usuario cuando se envía el formulario
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $newFirstName = isset($_POST['firstname']) ? $_POST['firstname'] : '';
    $newLastName = isset($_POST['lastname']) ? $_POST['lastname'] : '';
    $newEmail = isset($_POST['email']) ? $_POST['email'] : '';
    $oldPassword = isset($_POST['old_password']) ? $_POST['old_password'] : ''; // Obtener la contraseña antigua
    $newPassword = isset($_POST['password']) ? $_POST['password'] : '';

    // Validar que los datos no estén vacíos y que la contraseña antigua coincida
    if (empty($newFirstName) || empty($newLastName) || empty($newEmail)) {
        if ($oldPassword != $userData['password']) { // Verificar la contraseña antigua sea diferente a la nueva
            // Leer todo el archivo de usuarios y actualizar la información del usuario
            $newLines = [];
            foreach ($lines as $line) {
                $line = trim($line);
                if ($line === '') continue;

                $parts = explode(';', $line);
                if (count($parts) < 4) continue;

                list($storedEmail, $storedPass, $storedFirstName, $storedLastName) = $parts;

                if (strtolower($storedEmail) === strtolower($tempEmail)) {
                    // Reemplazar los datos con los nuevos
                    $storedPass = !empty($newPassword) ? $newPassword : $storedPass; // Actualizar contraseña solo si se ingresó
                    $newLines[] = "$newEmail;$storedPass;$newFirstName;$newLastName";
                    $_SESSION['user_session']['email'] = $newEmail; // Actualizar el email en la sesión si cambia
                } else {
                    $newLines[] = $line;
                }
            }

            // Guardar los datos actualizados en el archivo
            file_put_contents($userdataFile, implode("\n", $newLines) . "\n");
            echo "<script>alert('Datos actualizados con éxito');</script>";
            header('Location: profile.php'); // Recargar la página
            exit();
        } else {
            echo "<script>alert('La contraseña actual es incorrecta');</script>";
        }
    } else {
        echo "<script>alert('Por favor, completa todos los campos');</script>";
    }
}

?>

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Perfil</title>
    <link rel="stylesheet" href="resources/style.css">
</head>

<body>
    <div class="signup">
        <h1>Actualizar Datos</h1>
        <form id="profileForm" method="POST">
            <input type="text" name="firstname" id="firstname" placeholder="Nombre" required="required" />
            <input type="text" name="lastname" id="lastname" placeholder="Apellidos" required="required" />
            <input type="email" name="email" id="email" placeholder="Email" required="required" />
            <input type="password" name="password" id="password" placeholder="Nueva Contraseña (opcional)" />
            <button type="submit" class="btn btn-primary btn-block btn-large">Actualizar Datos</button><br \>
            <input type="password" name="old_password" id="old_password" placeholder="Contraseña actual para aplicar cambios" required="required" />
            <div class="button-container">
                <button type="button" class="btn btn-primary" onclick="window.history.back();">Volver</button>
                <button type="submit" class="btn-danger">Eliminar Cuenta</button>
            </div>
    <p id="error-message"></p>
        <p id="error-message"></p>
    </div>
</body>

</html>
