<?php
session_start();

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
    exit();
}

$email = strtolower($_SESSION['user_session']['email']);													//	Obtiene el 'email' del usuario y lo convierte a minúsculas
$name = explode('@', $email)[0];																			//	Obtiene el 'nombre' usando el 'email'
$last_name = '';																							//	Obtiene el 'last name' del usuario y lo convierte a minúsculas
$userDirectory = 'users/' . $email;																			//	Define el directorio del usuario
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
		$name = $storedFirstName;																			//	Obtener el 'nombre' establecido por el usuario
		$last_name = $storedLastName;																		//	Obtener el 'apellidos' establecido por el usuario
		break;
	}
}

$files = scandir($userDirectory);																			//	Lee los archivos dentro de la carpeta del usuario
$filesList = [];
foreach ($files as $file) {
    if ($file == '.' || $file == '..') continue;															//	Ignora los directorios '.' y '..'
    if (is_file($userDirectory . '/' . $file)) {															//	Verifica si es un archivo (no un directorio)
        $fileSize = filesize($userDirectory . '/' . $file);													//	Obtiene el tamaño del archivo
        $filesList[] = ['name' => $file, 'size' => formatFileSize($fileSize)];								//	Agrega la información del archivo a la lista
    }
}

header("HTTP/1.1 200 OK");

?>

<!-- -------------------------------------------- HTML -------------------------------------------- -->

<!DOCTYPE html>
<html lang="es">

<head>
	<meta charset="UTF-8">																														<!-- Define el tipo de caracteres utilizado en la página -->
	<meta name="viewport" content="width=device-width, initial-scale=1.0">																		<!-- Configura el viewport para adaptabilidad en móviles -->
	<title>Webserv 1.0</title>																													<!-- Título de la página -->
	<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">								<!-- Enlace a Font Awesome para usar iconos -->
	<link rel="stylesheet" href="resources/style.css">																							<!-- Enlace a la hoja de estilos -->
</head>

<body>
<header>
	<img src="resources/banner.jpg" alt="Banner" class="banner">																				<!-- Banner -->
	<h2 class="welcome-text">																													<!-- Bienvenida al usuario actual -->
		Bienvenido<br>
		<?php echo htmlspecialchars($name . ' ' . $last_name); ?>
	</h2>
	<a href="profile.php" class="icon-link" title="Modificar datos">																			<!-- Botón para modificar los datos del usuario (profile.php) -->
	<i class="fas fa-user-edit"></i>
	</a>

	<a href="logout.php" class="icon-link logout-link" title="Cerrar sesión">																	<!-- Botón para cerrar la sesión del usuario (logout.php) -->
	<i class="fas fa-sign-out-alt"></i>
	</a>
</header>

<hr class="separator">																															<!-- Separador bonito debajo del banner -->

<div class="center-container">
<div class="file-box scroll-container">																															<!-- Cuadro para la lista de archivos -->
	<div style="text-align: center; margin-bottom: 10px;">																						<!-- Título centrado sobre la tabla -->
   		<label class="file-box-title">Mis Archivos</label>
	</div>
	<table class="file-table">
		<thead>
			<tr>
				<th>Nombre</th>																													<!-- Columna para el 'Nombre' -->
				<th>Tamaño</th>																													<!-- Columna para el 'Tamaño' -->
				<th>Acciones</th>																												<!-- Columna para las 'Acciones' -->
			</tr>
		</thead>
		<tbody id="file-list">
			<?php foreach ($filesList as $file): ?>																								<!-- Recorre la lista de archivos -->
				<tr>
					<td><?php echo htmlspecialchars($file['name']); ?></td>																		<!-- Añade el 'Nombre' del archivo -->
					<td><?php echo $file['size']; ?></td>																						<!-- Añade el 'Tamaño' del archivo -->
					<td>
						<i class="fas fa-download action-icons" onclick="downloadFile('<?php echo $file['name']; ?>')" title="Descargar"></i>	<!-- Añade el icono 'Descargar' -->
						<i class="fas fa-trash-alt action-icons" onclick="deleteFile('<?php echo $file['name']; ?>')" title="Eliminar"></i>		<!-- Añade el icono 'Eliminar' -->
					</td>
				</tr>
			<?php endforeach; ?>																												<!-- Final del bucle for -->
		</tbody>
	</table>

	<input type="file" id="file-upload" multiple>																								<!-- Botón para elegir archivos -->
	<button id="upload-btn">Subir Archivos</button>																								<!-- Botón para subir los archivos elegidos -->

	<div id="progress-container">
		<progress id="upload-progress" value="0" max="100"></progress>																			<!-- Barra de progreso para la subida de archivos -->
		<button id="cancel-upload-btn">Cancelar Subida</button>																					<!-- Botón para cancelar la subida de archivos -->
	</div>
</div>

<div class="button-panel">
	<a target="_blank" href="evaluation.html">
		<button class="image-button">
			<img src="resources/evaluation.png" alt="Evaluación">
			<span>Evaluación</span>
		</button>
	</a>
	<a target="_blank" href="cgi/index.html">
		<button class="image-button">
			<img src="resources/cgi.png" alt="CGI">
			<span>CGI</span>
		</button>
	</a>
	<a target="_blank" href="/redirect">
		<button class="image-button">
			<img src="resources/redirect.png" alt="Redirecciones">
			<span>Redirecciones</span>
		</button>
	</a>
	<a target="_blank" href="/alias/">
		<button class="image-button">
			<img src="resources/alias.png" alt="Alias">
			<span>Alias</span>
		</button>
	</a>
	<a target="_blank" href="/directory/">
        <button class="image-button">
            <img src="resources/directory.png" alt="Directorios">
            <span>Directorios</span>
        </button>
    </a>
	<a target="_blank" href="/messages.php">
		<button class="image-button">
			<img src="resources/mesages.png" alt="Mensajes">
			<span>Mensajes</span>
		</button>
	</a>
	<a target="_blank" href="resources/Webserv.pdf">
		<button class="image-button">
			<img src="resources/documentation.png" alt="Documentación">
			<span>Documentación</span>
		</button>
	</a>
	<a href="contact.php">
		<button class="image-button">
			<img src="resources/contact.png" alt="Escríbenos">
			<span>Escríbenos</span>
		</button>
	</a>
    <a href="team/index.html">
        <button class="image-button">
            <img src="resources/team.png" alt="El equipo">
            <span>El Equipo</span>
        </button>
    </a>
</div>
</div>

<!-- ------------------------------------------- SCRIPT ------------------------------------------- -->

<script>

	// ||============= DOWNLOAD =============||

	function downloadFile(fileName) {																		//	Descargar archivo
		window.location.href = 'download.php?file=' + fileName;												//	Redirige a 'download.php'
	}

	// ||============== DELETE ==============||

	function deleteFile(fileName) {																			//	Eliminar archivo
		const xhr = new XMLHttpRequest();																	//	Crear un objeto 'XMLHttpRequest' para enviar una solicitud HTTP asíncrona al servidor

		xhr.open('DELETE', 'delete.php?file=' + fileName, true);											//	Configurar la solicitud GET asincrónica para eliminar el archivo

		xhr.onload = function() {																			//	Manejar la respuesta una vez que se complete la solicitud
			const response = JSON.parse(xhr.responseText);													//	Convertir la respuesta JSON en un objeto JavaScript
			if (response.status === 'success') {
				location.reload();																			//	Recarga la página para reflejar los cambios
			} else {
				alert('Error al eliminar el archivo: ' + response.message);									//	Muestra un mensaje de error si falla al eliminar el archivo
			}
		}

		xhr.onerror = function() {
			alert('Error al eliminar el archivo');															//	Muestra un mensaje de error si falla al eliminar el archivo
		}

		xhr.send();																							//	Enviar la solicitud al servidor	
	}

	// ||============== UPLOAD ==============||

	document.getElementById('upload-btn').addEventListener('click', function() {
		const fileInput = document.getElementById('file-upload');											//	Obtiene los archivos seleccionados
		const files = fileInput.files;

		if (files.length === 0) return;																		//	Si no se han seleccionado archivos, no hacer nada

		const progressBar = document.getElementById('upload-progress');										//	Obtiene la barra de progreso
		const progressContainer = document.getElementById('progress-container');							//	Obtiene el contenedor de la barra de progreso
		const cancelButton = document.getElementById('cancel-upload-btn');									//	Obtiene el botón de cancelar
		const uploadButton = document.getElementById('upload-btn');											//	Obtiene el botón de subir archivos
		const fileInputElement = document.getElementById('file-upload');									//	Obtiene la lista de archivos seleccionados	
		
		fileInputElement.disabled = true;																	//	Deshabilita el botón de elegir archivos
		uploadButton.disabled = true;																		//	Deshabilita el botón de subir archivos
		
		progressContainer.style.display = 'block';															//	Muestra la barra de progreso y el botón de cancelar
		
		let xhr = new XMLHttpRequest();																		//	Crear un objeto 'XMLHttpRequest' para enviar una solicitud HTTP asíncrona al servidor
		let formData = new FormData();																		//	Crea un objeto FormData para enviar los archivos seleccionados

		for (let i = 0; i < files.length; i++) {
			formData.append('files[]', files[i]);															//	Añadir los archivos seleccionados al formData
		}

		xhr.upload.onprogress = function(event) {															//	Evento para controlar el progreso de la subida
			if (event.lengthComputable) {
				const percentComplete = (event.loaded / event.total) * 100;
				progressBar.value = percentComplete;
			}
		}

		xhr.open('POST', 'upload.php', true);																//	Configurar la solicitud POST asincrónica para subir archivos

		xhr.onload = function() {																			//	Maneja el evento cuando la subida termina

			fileInputElement.disabled = false;																//	Habilita el botón de elegir archivos
			uploadButton.disabled = false;																	//	Habilita el botón de subir archivos
			progressContainer.style.display = 'none';														//	Oculta la barra de progreso y el botón de cancelar tras completar la subida
			progressBar.value = 0;																			//	Resetea la barra de progreso
			fileInput.value = '';																			//	Restablece el campo de archivos seleccionado

			if (xhr.status === 200) {
				location.reload();																			//	Recarga la página para reflejar los cambios
			} else {
				alert('Error en la subida de archivos');													//	Muestra un mensaje de error si falla la subida de archivos
			}
		}

		xhr.onerror = function() {																			//	Si ocurre un error en la solicitud, cancelar la subida y ocultar todo

			fileInputElement.disabled = false;																//	Habilita el botón de elegir archivos
			uploadButton.disabled = false;																	//	Habilita el botón de subir archivos
			progressContainer.style.display = 'none';														//	Oculta la barra de progreso y el botón de cancelar tras completar la subida
			progressBar.value = 0;																			//	Resetea la barra de progreso
			fileInput.value = '';																			//	Restablece el campo de archivos seleccionado

			alert('Error en la subida de archivos');
		}

		cancelButton.onclick = function() {
			xhr.abort();																					//	Aborta la solicitud actual

			fileInputElement.disabled = false;																//	Habilita el botón de elegir archivos
			uploadButton.disabled = false;																	//	Habilita el botón de subir archivos
			progressContainer.style.display = 'none';														//	Oculta la barra de progreso y el botón de cancelar tras completar la subida
			progressBar.value = 0;																			//	Resetea la barra de progreso
			fileInput.value = '';																			//	Restablece el campo de archivos seleccionado

			location.reload();																				//	Recarga la página para reflejar los cambios
		}

		xhr.send(formData);																					//	Enviar la solicitud al servidor	
	})

</script>

</body>
</html>
