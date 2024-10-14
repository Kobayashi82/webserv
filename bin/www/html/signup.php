<?php
session_start();

// Función para verificar si las credenciales almacenadas en el archivo userdata son válidas
function checkUserSession($username) {
    // Ubicación del archivo userdata
    $userdataFile = 'users/userdata';
    
    // Intentar abrir el archivo de usuarios
    $userdata = @file_get_contents($userdataFile);
    if ($userdata === false) {
        return false;
    }

    // Procesar las líneas del archivo de usuarios
    $lines = explode("\n", $userdata);
    foreach ($lines as $line) {
        $line = trim($line); // Eliminar espacios y saltos de línea innecesarios
        if ($line === '') {
            continue; // Ignorar líneas vacías
        }

        list($storedUser, $storedPass) = explode(';', $line);

        // Comprobar si el usuario coincide
        if ($storedUser == $username) {
            return true;
        }
    }
    return false;
}

// Verificar si la cookie de sesión existe y tiene un valor
if (isset($_COOKIE['user_session_cookie'])) {
    $username = $_COOKIE['user_session_cookie'];

    // Verificar si el nombre de usuario en la cookie es válido
    if (checkUserSession($username)) {
        // Iniciar sesión automáticamente si la cookie es válida
        $_SESSION['user_session'] = $username;
    } else {
        // Si la cookie no es válida, eliminarla
        setcookie('user_session_cookie', '', time() - 3600, "/");
        unset($_COOKIE['user_session_cookie']);
    }
}

// Si ya está logueado, redirigir
if (isset($_SESSION['user_session'])) {
    header('Location: /home.php');
    exit();
}

// Procesar datos del formulario solo si se envían por POST
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Leer los datos del cuerpo de la solicitud (stdin)
    parse_str(file_get_contents("php://input"), $postData);

    // Verificar si los datos están presentes
    $firstname = isset($postData['firstname']) ? $postData['firstname'] : '';
    $lastname = isset($postData['lastname']) ? $postData['lastname'] : '';
    $email = isset($postData['email']) ? $postData['email'] : '';
    $password = isset($postData['password']) ? $postData['password'] : '';
    $confirm_password = isset($postData['confirm_password']) ? $postData['confirm_password'] : '';

    // Definir la ruta del archivo userdata
    $userFolder = 'users';
    $userdataFile = $userFolder . '/userdata';
    
    // Verificar si la carpeta "users" existe, si no, crearla
    if (!is_dir($userFolder)) {
        if (!mkdir($userFolder, 0777, true)) {
            echo json_encode(['success' => false, 'message' => 'Unable to create users directory']);
            exit();
        }
    }

    // Verificar si el archivo userdata existe
    $userdata = @file_get_contents($userdataFile);
    if ($userdata === false) {
        // Si el archivo no existe, lo creamos y abrimos en modo escritura
        $file = fopen($userdataFile, 'w');
        if ($file === false) {
            echo json_encode(['success' => false, 'message' => 'Unable to create userdata file']);
            exit();
        }
        fclose($file);
        // Inicializamos la variable $userdata
        $userdata = '';
    }

    // Procesar las líneas del archivo de usuarios para verificar si el usuario ya existe
    $lines = explode("\n", $userdata);
    foreach ($lines as $line) {
        $line = trim($line); // Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue; // Ignorar líneas vacías

        list($storedUser, $storedPass) = explode(';', $line);

        // Comprobar si el usuario ya existe
        if (strtolower($storedUser) == strtolower($email)) {
            echo json_encode(['success' => false, 'message' => 'User already exists']);
            exit();
        }
    }

    // Verificar si las contraseñas coinciden
    if ($password !== $confirm_password) {
        echo json_encode(['success' => false, 'message' => 'Passwords do not match']);
        exit();
    }

    // Si el usuario no existe, añadir el nuevo usuario al archivo
    $newUser = $email . ';' . $password . "\n";
    $file = fopen($userdataFile, 'a');
    if ($file === false) {
        echo json_encode(['success' => false, 'message' => 'Unable to open userdata file']);
        exit();
    }
    fwrite($file, $newUser);
    fclose($file);

    echo json_encode(['success' => true, 'message' => 'User registered successfully']);
    exit();
}

?>

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>WebServ 1.0 - Sign Up</title>
  <link rel="stylesheet" href="./style.css">
</head>
<body>
<div class="signup">
  <h1>Sign Up</h1>
  <form id="signupForm" novalidate> <!-- Añadido novalidate para desactivar la validación nativa -->
      <input type="text" name="firstname" id="firstname" placeholder="First Name" required="required" />
      <input type="text" name="lastname" id="lastname" placeholder="Last Name" required="required" />
      <input type="email" name="email" id="email" placeholder="Email" required="required" />
      <input type="password" name="password" id="password" placeholder="Password" required="required" />
      <input type="password" name="confirm_password" id="confirm_password" placeholder="Confirm Password" required="required" />
      <button type="submit" class="btn btn-primary btn-block btn-large">Create Account</button>
  </form>
  <p style="text-align: center; margin-top: 15px; font-size: 12px; color: gray;">
      Already have an account? <a href="/login.php" style="color: yellow; margin-left: 5px;">Log In</a>
  </p>
  <p id="error-message"></p>
</div>

<script>
document.getElementById('signupForm').addEventListener('submit', function(e) {
    e.preventDefault(); // Prevenir el comportamiento por defecto del formulario
    
    // Validaciones personalizadas
    const firstname = document.getElementById('firstname');
    const lastname = document.getElementById('lastname');
    const email = document.getElementById('email');
    const password = document.getElementById('password');
    const confirm_password = document.getElementById('confirm_password');
    
    let formIsValid = true; // Variable para saber si el formulario es válido

    // Validación del primer nombre
    if (firstname.value === '') {
        firstname.setCustomValidity('Please enter your first name');
        formIsValid = false;
    } else {
        firstname.setCustomValidity('');
    }
    
    // Validación del apellido
    if (lastname.value === '') {
        lastname.setCustomValidity('Please enter your last name');
        formIsValid = false;
    } else {
        lastname.setCustomValidity('');
    }

    // Validación del email
    const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    if (email.value === '') {
        email.setCustomValidity('Please enter your email');
        formIsValid = false;
    } else if (!emailRegex.test(email.value)) { // Comprobamos si el email es válido con la expresión regular
        email.setCustomValidity('Please enter a valid email');
        formIsValid = false;
    } else {
        email.setCustomValidity('');
    }

    // Validación de la contraseña
    if (password.value === '') {
        password.setCustomValidity('Please enter your password');
        formIsValid = false;
    } else {
        password.setCustomValidity('');
    }

    // Validación de la confirmación de la contraseña
    if (confirm_password.value === '') {
        confirm_password.setCustomValidity('Please confirm your password');
        formIsValid = false;
    } else if (password.value !== confirm_password.value) {
        confirm_password.setCustomValidity('Passwords do not match');
        formIsValid = false;
    } else {
        confirm_password.setCustomValidity('');
    }

    // Si algún campo no es válido, no enviamos el formulario
    if (!formIsValid) {
        document.getElementById('signupForm').reportValidity();
        return; // Salir del submit
    }

    // Si la validación es correcta, enviamos el formulario usando Fetch API
    const formData = `firstname=${encodeURIComponent(firstname.value)}&lastname=${encodeURIComponent(lastname.value)}&email=${encodeURIComponent(email.value)}&password=${encodeURIComponent(password.value)}&confirm_password=${encodeURIComponent(confirm_password.value)}`;

    fetch('/signup.php', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: formData
    })
    .then(response => response.json()) // Procesar la respuesta como JSON
    .then(data => {
        if (data.success) {
            alert('Account created successfully');
            window.location.href = '/login.php';
        } else {
            alert(data.message);
        }
    })
    .catch(error => {
        console.error('Error:', error);
        alert('There was an error processing your request.');
    });
});
</script>
</body>
</html>