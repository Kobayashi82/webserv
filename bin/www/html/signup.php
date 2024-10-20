<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (isset($_SESSION['user_session'])) {
    header('Location: /home.php');																			//	Si está logueado, redirigir al 'home.php'
    exit();
}

// <!-- ------------------------------------------- POST ------------------------------------------- -->

if ($_SERVER['REQUEST_METHOD'] == 'POST') {																	//	Procesar datos del formulario solo si se envían por POST
    parse_str(file_get_contents("php://input"), $postData);													//	Leer los datos del cuerpo de la solicitud (stdin)

    $firstname = isset($postData['firstname']) ? $postData['firstname'] : '';								//	Obtener el valor de 'nombre'
    $lastname = isset($postData['lastname']) ? $postData['lastname'] : '';									//	Obtener el valor de 'apellidos'
    $email = isset($postData['email']) ? $postData['email'] : '';											//	Obtener el valor de 'email'
    $password = isset($postData['password']) ? $postData['password'] : '';									//	Obtener el valor de 'pass'
    $confirm_password = isset($postData['confirm_password']) ? $postData['confirm_password'] : '';			//	Obtener el valor de 'confirm_pass'

	if (strpos($email, '/') !== false) {																	//	Comprobar si el 'email' contiene el caracter prohibido "/"
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'El email no puede contener "/"']);				//	Si lo contiene, devolvemos un mensaje de "failed" al cliente
		exit();
	}

    $userdataFile = 'users/userdata';																		//	Definir la ruta del archivo 'userdata'
    
	if (!is_dir('users') && !mkdir('users', 0777, true)) {													//	Verificar si el directorio 'users' existe, si no, crea el directorio
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'Error interno al crear la cuenta']);			//	Si falla al crear el directorio, devolvemos un mensaje de "failed" al cliente
		exit();
	}

    $userdata = @file_get_contents($userdataFile);															//	Verificar si el archivo 'userdata' existe
    if ($userdata === false) {
        $file = fopen($userdataFile, 'w');																	//	Si 'userdata' no existe, lo creamos y abrimos en modo escritura
        if ($file === false) {
			header("HTTP/1.1 400 Bad Request");
            echo json_encode(['success' => false, 'message' => 'Error interno al crear la cuenta']);		//	Si falla al abrirlo, devolvemos un mensaje de "failed" al cliente
            exit();
        }
        fclose($file);																						//	Cerramos el archivo 'userdata'
    }

    $lines = explode("\n", $userdata);																		//	Procesar las líneas del archivo de usuarios
    foreach ($lines as $line) {
        $line = trim($line);																				//	Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue;																			//	Ignorar líneas vacías
		$parts = explode(';', $line);
		if (count($parts) < 4) continue;																		// Si no tiene las 4 partes, continuar con la siguiente línea

        list($storedUser, $storedPass, $storedFirstName, $storedLastName) = explode(';', $line);			//	Dividir la cadena en 'email', 'pass', 'firstname', y 'lastname'

        if (strtolower($storedUser) == strtolower($email)) {												//	Comprobar si el 'email' coincide
			header("HTTP/1.1 400 Bad Request");
            echo json_encode(['success' => false, 'message' => 'El e-mail pertenece a otro usuario']);		//	El 'email' ya existe, devolvemos un mensaje de "failed" al cliente
            exit();
        }
    }

    if ($password !== $confirm_password) {																	//	Comprobar si 'pass' y 'confirm_pass' coinciden
		header("HTTP/1.1 400 Bad Request");
        echo json_encode(['success' => false, 'message' => 'Las contraseñas no coinciden']);				//	No coinciden, devolvemos un mensaje de "failed" al cliente
        exit();
    }

    $newUser = $email . ';' . $password . ';' . $firstname . ';' . $lastname . ";\n";						//	Si el usuario no existe, añadimos el nuevo usuario al archivo
    $file = fopen($userdataFile, 'a');																		//	Abrimos 'userdata' en modo escritura
    if ($file === false) {
        echo json_encode(['success' => false, 'message' => 'Error interno al crear la cuenta']);			//	Si no se puede abrir 'userdata', enviar un mensaje de "failed" al cliente
        exit();
    }
    fwrite($file, $newUser);																				//	Guardamos el nuevo usuario
    fclose($file);																							//	Cerramos el archivo 'userdata'

	header("HTTP/1.1 201 Created");
    echo json_encode(['success' => true, 'message' => 'Usuario registrado con exito']);						//	Enviar un mesaje de "success" al cliente
    exit();
}
?>

<!-- -------------------------------------------- HTML -------------------------------------------- -->

<!DOCTYPE html>
<html lang="en">

<head>
	<meta charset="UTF-8">																												<!-- Define el tipo de caracteres utilizado en la página -->
	<meta name="viewport" content="width=device-width, initial-scale=1.0">																<!-- Configura el viewport para adaptabilidad en móviles -->
	<title>Registrarse</title>																											<!-- Título de la página -->
	<link rel="stylesheet" href="resources/style.css">																					<!-- Enlace a la hoja de estilos -->
</head>

<body>
<div class="signup">																													<!-- Contenedor principal -->
	<h1>Crea tu cuenta</h1>																												<!-- Título de la sección -->
	<form id="signupForm">
		<input type="text" name="firstname" id="firstname" placeholder="Nombre" required="required" />									<!-- Campo para ingresar el 'nombre' -->
		<input type="text" name="lastname" id="lastname" placeholder="Apellidos" required="required" />									<!-- Campo para ingresar el 'apellidos' -->
		<input type="email" name="email" id="email" placeholder="Email" required="required" />											<!-- Campo para ingresar el 'email' -->
		<input type="password" name="password" id="password" placeholder="Contraseña" required="required" />							<!-- Campo para ingresar el 'pass' -->
		<input type="password" name="confirm_password" id="confirm_password" placeholder="Confirmar contraseña" required="required" />	<!-- Campo para ingresar el 'confirm_pass' -->
		<button type="submit" class="btn btn-primary btn-block btn-large">Crear Cuenta</button>											<!-- Botón para enviar el formulario -->
	</form>

	<p style="text-align: left; margin-top: 15px; font-size: 12px; color: gray; margin-left: 50px;">
		¿Ya tienes una cuenta? <a href="/login.php" style="color: #C0C000; margin-left: 15px;">Iniciar sesión</a>						<!-- Enlace a la página de registro -->
    <br/>
		¿Necesitas ayuda?<a href="/contact.php" style="color: #C0C000; margin-left: 42px;">Contáctanos</a>								<!-- Enlace para la página de contacto -->
	</p>
	<p id="error-message"></p>																											<!-- Elemento para mostrar mensajes de error -->
</div>

<!-- ------------------------------------------- SCRIPT ------------------------------------------- -->

<script>

	document.getElementById('signupForm').addEventListener('submit', function(e) {
		e.preventDefault();																					//	Previene el envío del formulario por defecto
		
		const firstname = document.getElementById('firstname');												//	Obtiene el valor de 'nombre'
		const lastname = document.getElementById('lastname');												//	Obtiene el valor de 'apellidos'
		const email = document.getElementById('email');														//	Obtiene el valor de 'email'
		const password = document.getElementById('password');												//	Obtiene el valor de 'pass'
		const confirm_password = document.getElementById('confirm_password');								//	Obtiene el valor de 'confirm_pass'
		
		fetch('/signup.php', {																				//	Realiza la solicitud a 'signup.php' con los datos del formulario, osea, (POST a signup.php)
			method: 'POST',
			headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
			body: `firstname=${encodeURIComponent(firstname.value)}&` +
				`lastname=${encodeURIComponent(lastname.value)}&` +
				`email=${encodeURIComponent(email.value)}&` +
				`password=${encodeURIComponent(password.value)}&` +
				`confirm_password=${encodeURIComponent(confirm_password.value)}`
		})

		.then(response => response.json())																	//	Convierte la respuesta en formato JSON para poder acceder a los datos

		.then(data => {																						//	Maneja la respuesta del servidor
			if (data.success) {
				alert('Cuenta creada con éxito');															//	Muestra una alerta de "success"
				window.location.href = '/login.php';														//	Redirige a 'login.php' en caso de éxito
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
