<?php
session_start();																							//	Inicia una nueva sesión o reanuda la sesión existente

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
    exit();
}

$email = $_SESSION['user_session']['email'];																//	Obtiene el 'email' del usuario y lo convierte a minúsculas
$pass = '';																									//	Obtiene el 'pass'
$name = '';																									//	Obtiene el 'nombre'
$last_name = '';																							//	Obtiene el 'apellido'

$userDirectory = 'users/' . strtolower($email);																//	Define el directorio del usuario
createUserDirectory($userDirectory);																		//	Crea el directorio si no existe

$userdata = @file_get_contents('users/userdata');															//	Verificar si el archivo 'userdata' existe
$lines = explode("\n", $userdata);																			//	Procesar las líneas del archivo de usuarios
foreach ($lines as $line) {
	$name = $line;
	$line = trim($line);																					//	Eliminar espacios y saltos de línea innecesarios
	if ($line === '') continue;																				//	Ignorar líneas vacías
    if (count(explode(';', $line)) < 4) continue;															// Si no tiene las 4 partes, continuar con la siguiente línea

	list($storedUser, $storedPass, $storedFirstName, $storedLastName) = explode(';', $line);				//	Dividir la cadena en 'email', 'pass', 'firstname', y 'lastname'

	if (strtolower($storedUser) == strtolower($email)) {													//	Comprobar si el 'email' coincide
		$pass = $storedPass;																				//	Obtener el 'pass' establecido por el usuario
		$name = $storedFirstName;																			//	Obtener el 'nombre' establecido por el usuario
		$last_name = $storedLastName;																		//	Obtener el 'apellido' establecido por el usuario
		break;
	}
}

// <!-- ------------------------------------------- POST ------------------------------------------- -->

if ($_SERVER['REQUEST_METHOD'] == 'POST') {																	//	Procesar datos del formulario solo si se envían por POST
    parse_str(file_get_contents("php://input"), $postData);													//	Leer los datos del cuerpo de la solicitud (stdin)

	$userdataFile = 'users/userdata';																		//	Definir la ruta del archivo 'userdata'

	if (isset($postData['action']) && $postData['action'] === 'DELETE') {
		$old_password = isset($postData['old_password']) ? $postData['old_password'] : '';					//	Obtener el valor de 'old_pass'

		if ($pass !== $old_password) {
			header("HTTP/1.1 400 Bad Request");
			echo json_encode(['success' => false, 'message' => 'Contraseña no válida']);					//	Contraseña inválida, devolvemos un mensaje de "failed" al cliente
			exit();
		}

		deleteDirectory($userDirectory);																	//	Elimina el directorio

		$userdata = @file_get_contents($userdataFile);														//	Verificar si el archivo 'userdata' existe
		if ($userdata !== false) {
				
			$newLines = [];																					//	Nuevo array para las líneas sin el usuario a eliminar
			
			$lines = explode("\n", $userdata);																//	Procesar las líneas del archivo de usuarios
			foreach ($lines as $line) {
				$line = trim($line);																		//	Eliminar espacios y saltos de línea innecesarios
				if ($line === '') continue;																	//	Ignorar líneas vacías
				$parts = explode(';', $line);
				if (count($parts) < 4) continue;															//	Si no tiene las 4 partes, continuar con la siguiente línea
				
				list($storedUser, $storedPass, $storedFirstName, $storedLastName) = explode(';', $line);	//	Dividir la cadena en 'email', 'pass', 'firstname', y 'lastname'
				
				if (strtolower($storedUser) == strtolower($email)) continue;								//	No añadir esta línea al array
				$newLines[] = $line;																		//	Añade la linea al array de lines
			}
			
			file_put_contents($userdataFile, implode("\n", $newLines));										//	Guardar las líneas actualizadas en el archivo
		}

		header("HTTP/1.1 200 OK");
        echo json_encode(array('success' => true, 'message' => 'Cuenta eliminada con exito'));
        exit();
    }

	$n_firstname = isset($postData['firstname']) ? $postData['firstname'] : '';								//	Obtener el valor de 'nombre'
    $n_lastname = isset($postData['lastname']) ? $postData['lastname'] : '';								//	Obtener el valor de 'apellidos'
    $n_email = isset($postData['email']) ? $postData['email'] : '';											//	Obtener el valor de 'email'
    $n_password = isset($postData['password']) ? $postData['password'] : '';								//	Obtener el valor de 'pass'
    $old_password = isset($postData['old_password']) ? $postData['old_password'] : '';						//	Obtener el valor de 'old_pass'


	if (strpos($n_email, '/') !== false) {																	//	Comprobar si el 'email' contiene el caracter prohibido "/"
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'El email no puede contener "/"']);				//	Si lo contiene, devolvemos un mensaje de "failed" al cliente
		exit();
	}
  
	if (!is_dir('users') && !mkdir('users', 0777, true)) {													//	Verificar si el directorio 'users' existe, si no, crea el directorio
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'Error interno']);								//	Si falla al crear el directorio, devolvemos un mensaje de "failed" al cliente
		exit();
	}

    $userdata = @file_get_contents($userdataFile);															//	Verificar si el archivo 'userdata' existe
    if ($userdata === false) {
        $file = fopen($userdataFile, 'w');																	//	Si 'userdata' no existe, lo creamos y abrimos en modo escritura
        if ($file === false) {
			header("HTTP/1.1 400 Bad Request");
            echo json_encode(['success' => false, 'message' => 'Error interno']);							//	Si falla al abrirlo, devolvemos un mensaje de "failed" al cliente
            exit();
        }
        fclose($file);																						//	Cerramos el archivo 'userdata'
    }

	if ($pass !== $old_password) {																			//	Comprobar si 'pass' y 'old_pass' no coinciden
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'Las contraseñas actual es incorrecta']);		//	Contraseña inválida, devolvemos un mensaje de "failed" al cliente
		exit();
	}

	$newLines = [];																							//	Nuevo array para las líneas sin el usuario a eliminar
	$userFound = false;																						//	Variable para marcar si el usuario fue encontrado

    $lines = explode("\n", $userdata);																		//	Procesar las líneas del archivo de usuarios
    foreach ($lines as $line) {
        $line = trim($line);																				//	Eliminar espacios y saltos de línea innecesarios
        if ($line === '') continue;																			//	Ignorar líneas vacías
		$parts = explode(';', $line);
		if (count($parts) < 4) continue;																	//	Si no tiene las 4 partes, continuar con la siguiente línea
		
        list($storedUser, $storedPass, $storedFirstName, $storedLastName) = explode(';', $line);			//	Dividir la cadena en 'email', 'pass', 'firstname', y 'lastname'

		if (strtolower($storedUser) == strtolower($email)) {
			$userFound = true;																				//	Usuario encontrado
			continue;																						//	No añadir esta línea al array
		}
        if (strtolower($storedUser) == strtolower($n_email)) {												//	Comprobar si el 'email' coincide
			header("HTTP/1.1 400 Bad Request");
            echo json_encode(['success' => false, 'message' => 'El e-mail pertenece a otro usuario']);		//	El 'email' ya existe, devolvemos un mensaje de "failed" al cliente
            exit();
        }
		$newLines[] = $line;																				//	Añade la linea al array de lines
    }

	if (!$userFound) {
		header("HTTP/1.1 400 Bad Request");
		echo json_encode(['success' => false, 'message' => 'Usuario no encontrado']);						//	No existe el usuario, devolvemos un mensaje de "failed" al cliente
		exit();
	}

	if (empty($n_password)) $n_password = $pass;
    $newUser = $n_email . ';' . $n_password . ';' . $n_firstname . ';' . $n_lastname . ";\n";				//	Si el usuario no existe, añadimos el nuevo usuario al archivo

	$newLines[] = $newUser;																					//	Añadir la nueva línea al array de líneas
    file_put_contents($userdataFile, implode("\n", $newLines));												//	Guardar las líneas actualizadas en el archivo

	if (is_dir($userDirectory)) {
		$newUserDirectory = 'users/' . strtolower($n_email);												//	Ruta de la nueva carpeta
		rename($userDirectory, $newUserDirectory);															//	Cambia el nombre la carpera
	}

	$_SESSION['user_session'] = array('email' => $n_email, 'pass' => $n_password);							//	Crear la sesión
	if (isset($_COOKIE['user_session_cookie'])) {
		$cookie_value = base64_encode($n_email . ';' . $n_password . ';');									//	Crear cadena con 'email' y 'pass' separadas por ";"
		setcookie('user_session_cookie', $cookie_value, time() + (7 * 24 * 60 * 60), "/", "", false, true);				//	Crear cookie para recordar al usuario por 7 días
	}

	header("HTTP/1.1 200 OK");
    echo json_encode(['success' => true, 'message' => 'Usuario registrado con exito']);						//	Enviar un mesaje de "success" al cliente
    exit();
}

?>

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">																																		<!-- Define el tipo de caracteres utilizado en la página -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0">																						<!-- Configura el viewport para adaptabilidad en móviles -->
    <title>Perfil</title>																																		<!-- Título de la página -->
    <link rel="stylesheet" href="resources/style.css">																											<!-- Enlace a la hoja de estilos -->
</head>

<body>
<div class="signup">																																			<!-- Contenedor principal -->
    <h1>Actualizar Datos</h1>																																	<!-- Título de la sección -->
    <form id="profileForm">
        <input type="text" name="firstname" id="firstname" placeholder="Nombre" value="<?php echo htmlspecialchars($name); ?>" required="required" />			<!-- Campo para ingresar el 'nombre' -->
        <input type="text" name="lastname" id="lastname" placeholder="Apellidos" value="<?php echo htmlspecialchars($last_name); ?>" required="required" />		<!-- Campo para ingresar el 'apellidos' -->
        <input type="email" name="email" id="email" placeholder="Email" value="<?php echo htmlspecialchars($email); ?>" required="required" />					<!-- Campo para ingresar el 'email' -->
        <input type="password" name="password" id="password" placeholder="Nueva Contraseña (opcional)" />														<!-- Campo para ingresar el 'nueva pass' -->
        <button type="submit" class="btn btn-primary btn-block btn-large">Actualizar Datos</button><br \>														<!-- Botón para enviar el formulario -->
        <input type="password" name="old_password" id="old_password" placeholder="Contraseña actual para aplicar cambios" required="required" />				<!-- Campo para ingresar el 'old pass' -->
        <div class="button-container">
            <button type="button" class="btn btn-primary" onclick="window.history.back();">Volver</button>														<!-- Botón para volver atras -->
            <button type="button" id="deleteAccount" class="btn-danger">Eliminar Cuenta</button>																<!-- Botón para eliminar la cuenta -->
        </div>
	</form>

	<p style="text-align: left; margin-top: 15px; font-size: 12px; color: gray; margin-left: 50px;">
		¿Necesitas ayuda?<a href="/contact.php" style="color: #C0C000; margin-left: 42px;">Contáctanos</a>														<!-- Enlace para la página de contacto -->
	</p>
	<p id="error-message"></p>																																	<!-- Elemento para mostrar mensajes de error -->
</div>

<!-- ------------------------------------------- SCRIPT ------------------------------------------- -->

<script>

	document.getElementById('profileForm').addEventListener('submit', function(e) {
		e.preventDefault();																					//	Previene el envío del formulario por defecto
		
		const firstname = document.getElementById('firstname');												//	Obtiene el valor de 'nombre'
		const lastname = document.getElementById('lastname');												//	Obtiene el valor de 'apellidos'
		const email = document.getElementById('email');														//	Obtiene el valor de 'email'
		const password = document.getElementById('password');												//	Obtiene el valor de 'pass'
		const old_password = document.getElementById('old_password');										//	Obtiene el valor de 'old_pass'
		
		fetch('/profile.php', {																				//	Realiza la solicitud a 'signup.php' con los datos del formulario, osea, (POST a signup.php)
			method: 'POST',
			headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
			body: `firstname=${encodeURIComponent(firstname.value)}&` +
				`lastname=${encodeURIComponent(lastname.value)}&` +
				`email=${encodeURIComponent(email.value)}&` +
				`password=${encodeURIComponent(password.value)}&` +
				`old_password=${encodeURIComponent(old_password.value)}`
		})

		.then(response => response.json())																	//	Convierte la respuesta en formato JSON para poder acceder a los datos

		.then(data => {																						//	Maneja la respuesta del servidor
			if (data.success) {
				alert('Datos actualizados con éxito');														//	Muestra una alerta de "success"
				window.location.href = '/home.php';															//	Redirige a 'login.php' en caso de éxito
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

	document.getElementById('deleteAccount').addEventListener('click', function(e) {
        e.preventDefault();																					//	Previene el envío del formulario por defecto

		const old_password = document.getElementById('old_password');										//	Obtiene el valor de 'old_pass'

        if (confirm('¿Estás seguro de que quieres eliminar tu cuenta?')) {									//	Confirmación antes de eliminar
            fetch('/profile.php', {																			//	Realiza la solicitud a 'profile.php' con el mensaje "DELETE"
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: `action=DELETE&old_password=${encodeURIComponent(old_password.value)}`				//	Solo envía el mensaje "DELETE"
            })

			.then(response => response.json())																//	Convierte la respuesta en formato JSON para poder acceder a los datos

			.then(data => {																					//	Maneja la respuesta del servidor
                if (data.success) {
                    alert('Cuenta eliminada con éxito');													//	Muestra una alerta de "success"
                    window.location.href = '/logout.php';													//	Redirige a logout o cualquier página de confirmación
                } else {
                    const errorMessage = document.getElementById('error-message');
                    errorMessage.textContent = data.message;												//	Muestra un mensaje de error debajo del botón
                }
            })

			.catch(error => {																				//	Maneja los errores de la solicitud
				console.error('Error:', error);																//	Imprime el error en la consola
				alert('Se produjo un error al procesar tu solicitud');										//	Muestra una alerta al usuario
			})
        }
    })

</script>
</body>
</html>
