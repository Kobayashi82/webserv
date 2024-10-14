<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (isset($_SESSION['user_session'])) {																		//	Si ya está logueado, redirigir a 'home.php'
    header('Location: /home.php');
    exit();
}

// <!-- ------------------------------------------- POST ------------------------------------------- -->

if ($_SERVER['REQUEST_METHOD'] == 'POST') {																	//	Procesar datos del formulario solo si se envían por POST
    parse_str(file_get_contents("php://input"), $postData);													//	Leer los datos del cuerpo de la solicitud (stdin)

    $email = isset($postData['u']) ? $postData['u'] : '';													//	Obtener el valor de 'email'
    $pass = isset($postData['p']) ? $postData['p'] : '';													//	Obtener el valor de 'pass'

    $userdata = @file_get_contents('users/userdata');														//	Verificar si el archivo 'userdata' existe
    if ($userdata === false) {
        echo json_encode(['success' => false, 'message' => 'Email o contraseña no válidos']);				//	Enviar un mesaje de "failed" al cliente
        exit();
    }

    $lines = explode("\n", $userdata);																		//	Procesar las líneas del archivo de usuarios
    foreach ($lines as $line) {
        $line = trim($line);																				//	Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue;																			//	Ignorar líneas vacías

        list($storedUser, $storedPass) = explode(';', $line);												//	Dividir la cadena en 'email' y 'pass'

        if (strtolower($storedUser) == strtolower($email) && $storedPass == $pass) {						//	Comprobar si el 'email' y 'pass' coincide
            $_SESSION['user_session'] = $email;																//	Crear la sesión

			if (isset($postData['remember_me']) && $postData['remember_me'] == '1') {						//	Verificar si el usuario ha marcado la opción "Recordarme"
				$cookie_value = base64_encode($email . ';' . $pass);										//	Crear cadena con 'email' y 'pass' separadas por ";"
				setcookie('user_session_cookie', $cookie_value, time() + (7 * 24 * 60 * 60), "/", "");		//	Crear cookie para recordar al usuario por 7 días
			} else {
				if (isset($_COOKIE['user_session_cookie'])) {												//	Si no se ha marcado la opción "Recordarme"
					setcookie('user_session_cookie', '', time() - 3600, "/");								//	Eliminar la cookie del cliente
					unset($_COOKIE['user_session_cookie']);													//	Eliminar la cookie del entorno
				}
			}

            echo json_encode(['success' => true, 'message' => 'Sesión inciada con éxito']);					//	Enviar un mesaje de "success" al cliente
            exit();
        }
    }

    echo json_encode(['success' => false, 'message' => 'Email o contraseña no válidos']);					// Si no coincide ningún usuario, enviar un mensaje de "failed" al cliente
	exit();
}
?>

<!-- -------------------------------------------- HTML -------------------------------------------- -->

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">																					<!-- Define el tipo de caracteres utilizado en la página -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0">									<!-- Configura el viewport para adaptabilidad en móviles -->
    <title>Iniciar sesión</title>																			<!-- Título de la página -->
    <link rel="stylesheet" href="resources/style.css">														<!-- Enlace a la hoja de estilos -->
</head>

<body>
<div class="login">																							<!-- Contenedor principal -->
    <h1>Iniciar sesión</h1>																					<!-- Título de la sección -->
    <form id="loginForm">
        <input type="email" name="u" id="username" placeholder="Email" required="required" />				<!-- Campo para ingresar el 'email' -->
        <input type="password" name="p" id="password" placeholder="Contraseña" required="required" />		<!-- Campo para ingresar el 'pass' -->
        
        <label class="remember-me">																			<!-- Etiqueta para la opción "Recordarme" -->
            <input type="checkbox" name="remember_me" value="1"> Recordarme									<!-- Checkbox para recordar al usuario -->
        </label>

        <button type="submit" class="btn btn-primary btn-block btn-large">Continuar</button>				<!-- Botón para enviar el formulario -->
    </form>

    <p style="text-align: left; margin-top: 15px; font-size: 12px; color: gray; margin-left: 50px;">
		¿No tienes cuenta? <a href="/signup.php" style="color: #C0C000; margin-left: 15px;">Regístrate</a>	<!-- Enlace a la página de registro -->
    <br />
		¿Necesitas ayuda? <a href="/contact.php" style="color: #C0C000; margin-left: 19px;">Contáctanos</a>	<!-- Enlace para la página de contacto -->
    </p>
    <p id="error-message"></p>																				<!-- Elemento para mostrar mensajes de error -->
</div>

<!-- ------------------------------------------- SCRIPT ------------------------------------------- -->

<script>

	document.getElementById('loginForm').addEventListener('submit', function(e) {
		e.preventDefault();																					//	Previene el envío del formulario por defecto
		
		const username = document.getElementById('username');												//	Obtiene el valor de 'email'
		const password = document.getElementById('password');												//	Obtiene el valor de 'pass'
		const rememberMe = document.querySelector('input[name="remember_me"]').checked ? '1' : '0';			//	Obtiene el valor de 'Recordarme'

		fetch('/login.php', {																				//	Realiza la solicitud a 'login.php' con los datos del formulario, osea, (POST a login.php)
			method: 'POST',
			headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
			body: `u=${encodeURIComponent(username.value)}&` +
				  `p=${encodeURIComponent(password.value)}&` +
				  `remember_me=${encodeURIComponent(rememberMe)}`
		})

		.then(response => response.json())																	//	Convierte la respuesta en formato JSON para poder acceder a los datos

		.then(data => {																						//	Maneja la respuesta del servidor
			if (data.success) {
				window.location.href = '/home.php';															//	Redirige a 'home.php' en caso de éxito
			} else {
				const errorMessage = document.getElementById('error-message');
				errorMessage.textContent = data.message;													//	Muestra un mensaje de error debajo del botón
			}
		})

		.catch(error => {																					//	Maneja los errores de la solicitud
			console.error('Error:', error);																	//	Imprime el error en la consola
			alert('Se produjo un error al procesar tu solicitud');											//	Muestra una alerta al usuario
		})

	})

</script>

</body>
</html>
