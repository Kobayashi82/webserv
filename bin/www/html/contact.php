<?php

include('functions.php');																					//	Incluye el archivo de funciones

if ($_SERVER['REQUEST_METHOD'] == 'POST') {																	//	Procesar datos del formulario solo si se envían por POST
    parse_str(file_get_contents("php://input"), $postData);													//	Leer los datos del cuerpo de la solicitud (stdin)

    $name = isset($postData['name']) ? trim($postData['name']) : '';										//	Obtener el valor de 'nombre'
    $email = isset($postData['email']) ? trim($postData['email']) : '';										//	Obtener el valor de 'email'
    $subject = isset($postData['subject']) ? trim($postData['subject']) : '';								//	Obtener el valor de 'asunto'
    $message = isset($postData['message']) ? trim($postData['message']) : '';								//	Obtener el valor de 'mensaje'

    if (!empty($name) && !empty($email) && !empty($subject) && !empty($message)) {

        $msgDirectory = 'messages';																			//	Establece la carpeta de los mensajes
		createUserDirectory($msgDirectory);																	//	Crea el directorio si no existe

        $filePath = $msgDirectory . '/' . uniqid('message_') . '.txt';										//	Ruta completa del archivo del mensaje
        $messageContent = "Name: $name\nEmail: $email\nSubject: $subject\nMessage: $message\n\n";			//	Contenido del mensaje
        file_put_contents($filePath, $messageContent);														//	Crea el archivo del mensaje

        echo json_encode(['success' => true, 'message' => 'Mensaje enviado con éxito']);					//	Enviar un mesaje de "success" al cliente
    } else {
		echo json_encode(['success' => false, 'message' => 'No se pudo enviar el mensaje']);				//	Si no se pudo crear, enviar un mensaje de "failed" al cliente
    }

	exit();
}
?>

<!-- -------------------------------------------- HTML -------------------------------------------- -->

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">																											<!-- Define el tipo de caracteres utilizado en la página -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0">															<!-- Configura el viewport para adaptabilidad en móviles -->
    <title>Contacto</title>																											<!-- Título de la página -->
    <link rel="stylesheet" href="resources/style.css">																				<!-- Enlace a la hoja de estilos -->
</head>

<body>
<div class="signup">																												<!-- Contenedor principal -->
    <h1>Escríbenos</h1>																												<!-- Título de la sección -->
    <form id="contactForm" method="post">
        <input type="text" name="name" id="name" placeholder="Nombre" required="required" />										<!-- Campo para ingresar el 'nombre' -->
        <input type="email" name="email" id="email" placeholder="Email" required="required" />										<!-- Campo para ingresar el 'email' -->
        <input type="text" name="subject" id="subject" placeholder="Asunto" required="required" />									<!-- Campo para ingresar el 'asunto' -->
        <textarea name="message" id="message" placeholder="Mensaje" required="required" rows="5"></textarea>						<!-- Campo para ingresar el 'mensaje' -->
        <button type="submit" class="btn btn-primary btn-block btn-large">Enviar Mensaje</button>									<!-- Botón para enviar el formulario -->
    </form>

    <p style="text-align: left; margin-top: 15px; font-size: 12px; color: gray; margin-left: 0px;">
		¿Necesitas asistencia de inmediato? <a href="/support.html" style="color: #C0C000; margin-left: 3px;">Obtener Soporte</a>	<!-- Enlace a la página de soporte -->
    <br />
		¿Volver a la página principal? <a href="/home.php" style="color: #C0C000; margin-left: 42px;">Página Principal</a>			<!-- Enlace para la página principal -->
    </p>
</div>

<!-- ------------------------------------------- SCRIPT ------------------------------------------- -->

<script>

	document.getElementById('contactForm').addEventListener('submit', function(e) {
		e.preventDefault();																					//	Previene el envío del formulario por defecto
		
		const name = document.getElementById('name');														//	Obtiene el valor de 'nombre'
		const email = document.getElementById('email');														//	Obtiene el valor de 'email'
		const subject = document.getElementById('subject');													//	Obtiene el valor de 'asunto'
		const message = document.getElementById('message');													//	Obtiene el valor de 'mensaje'

		fetch('/contact.php', {																				//	Realiza la solicitud a 'contact.php' con los datos del formulario, osea, (POST a contact.php)
			method: 'POST',
			headers: {
				'Content-Type': 'application/x-www-form-urlencoded',
			},
			body: `name=${encodeURIComponent(name.value)}&` +
				  `email=${encodeURIComponent(email.value)}&` +
				  `subject=${encodeURIComponent(subject.value)}&` +
				  `message=${encodeURIComponent(message.value)}`
		})

		.then(response => response.json())																	//	Convierte la respuesta en formato JSON para poder acceder a los datos

		.then(data => {																						//	Maneja la respuesta del servidor
			if (data.success) {
				alert('Message sent successfully!');														//	Muestra una alerta de "success"
			} else {
				alert('Message sent failed!');																//	Muestra un mensaje de error debajo del botón
			}
		})

		.catch(error => {																					//	Maneja los errores de la solicitud
			console.error('Error:', error);																	//	Imprime el error en la consola
			alert('Se produjo un error al procesar tu solicitud');											//	Muestra una alerta al usuario
		})

		name.value = '';
		email.value = '';
		subject.value = '';
		message.value = '';
	})

</script>
</body>
</html>
