<?php
session_start();

include('functions.php');																					//	Incluye el archivo de funciones

UserSession();																								//	Verifica si la sesión del usuario ya está activa a través de la cookie y la inicia si es válida

if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');																			//	Si no existe la cookie, redirigir al 'login.php'
    exit();
}






$email = strtolower($_SESSION['user_session']);																//	Obtiene el 'email' del usuario y lo convierte a minúsculas
$userDirectory = 'users/' . $email;																			//	Define el directorio del usuario
createUserDirectory($userDirectory);																		//	Crea el directorio si no existe

// Lee los archivos dentro de la carpeta del usuario
$files = scandir($userDirectory);
$filesList = [];
foreach ($files as $file) {
    // Ignora las carpetas '.' y '..'
    if ($file == '.' || $file == '..') continue;

    // Verifica si es un archivo (no una carpeta)
    if (is_file($userDirectory . '/' . $file)) {
        // Obtiene el tamaño del archivo en bytes
        $fileSize = filesize($userDirectory . '/' . $file);

        // Agrega la información del archivo a la lista, usando la función para formatear el tamaño
        $filesList[] = ['name' => $file, 'size' => formatFileSize($fileSize)];
    }
}
?>

<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Webserv 1.0</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">	<!-- Enlace a Font Awesome para usar iconos -->
  <link rel="stylesheet" href="resources/style.css">
  <style>
    /* Estilo personalizado para los iconos de acción */
    .action-icons {
      cursor: pointer;
      font-size: 1.2em; /* Tamaño de los iconos */
      margin-right: 10px;
      color: #333;
    }

    .action-icons:hover {
      color: #007bff; /* Cambia el color al pasar el mouse */
    }

    /* Estilo de la barra de progreso */
    #progress-container {
      margin-top: 10px;
      display: none;
    }

    #upload-progress {
      width: 100%;
      height: 20px;
    }

    #cancel-upload-btn {
      margin-left: 10px;
      background-color: red;
      color: white;
      padding: 5px 10px;
      border: none;
      cursor: pointer;
    }

    #cancel-upload-btn:hover {
      background-color: darkred;
    }
  </style>
</head>
<body>
  <header>
    <img src="resources/banner.jpg" alt="Banner" class="banner">

  <a href="profile.php" class="icon-link" title="Modificar datos">
    <i class="fas fa-user-edit"></i>
  </a>

  <a href="logout.php" class="icon-link logout-link" title="Cerrar sesión">
    <i class="fas fa-sign-out-alt"></i>
  </a>
  </header>

  <!-- Separador bonito debajo del banner -->
  <hr class="separator">

  <!-- Cuadro para la lista de archivos -->
  <div class="file-box">
     <!-- Título centrado sobre la tabla -->
  <div style="text-align: center; margin-bottom: 10px;">
    <label class="file-box-title">Mis Archivos</label>
  </div>
    <table class="file-table">
      <thead>
        <tr>
          <th>Nombre</th>
          <th>Tamaño</th>
          <th>Acciones</th>
        </tr>
      </thead>
      <tbody id="file-list">
        <?php foreach ($filesList as $file): ?>
          <tr>
            <td><?php echo htmlspecialchars($file['name']); ?></td>
            <td><?php echo $file['size']; ?></td>
            <td>
              <i class="fas fa-download action-icons" onclick="downloadFile('<?php echo $file['name']; ?>')" title="Descargar"></i>
              <i class="fas fa-trash-alt action-icons" onclick="deleteFile('<?php echo $file['name']; ?>')" title="Eliminar"></i>
            </td>
          </tr>
        <?php endforeach; ?>
      </tbody>
    </table>

    <!-- Botón para añadir archivos -->
    <input type="file" id="file-upload" multiple>
    <button id="upload-btn">Subir Archivos</button>

    <!-- Barra de progreso para la subida de archivos -->
    <div id="progress-container">
      <progress id="upload-progress" value="0" max="100"></progress>
      <button id="cancel-upload-btn">Cancelar Subida</button>
    </div>
  </div>

  <script>
  // Descargar archivo
  function downloadFile(fileName) {
    window.location.href = 'download.php?file=' + fileName;
  }

  // Eliminar archivo y recargar la página
  function deleteFile(fileName) {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', 'delete.php?file=' + fileName, true);
    xhr.onload = function() {
        const response = JSON.parse(xhr.responseText);
        if (response.status === 'success') {
            location.reload();  // Recarga la página para reflejar los cambios
        } else {
            alert('Error al eliminar el archivo: ' + response.message);  // Muestra el mensaje de error solo si la eliminación falla
        }
    };

    xhr.onerror = function() {
        alert('Error al eliminar el archivo');
    };

    xhr.send();
  }

  document.getElementById('upload-btn').addEventListener('click', function() {
    const fileInput = document.getElementById('file-upload');
    const files = fileInput.files;

    if (files.length === 0) return;

    const progressBar = document.getElementById('upload-progress');
    const progressContainer = document.getElementById('progress-container');
    const cancelButton = document.getElementById('cancel-upload-btn');
    const uploadButton = document.getElementById('upload-btn');
    const fileInputElement = document.getElementById('file-upload');
    
    // Deshabilitar seleccionar y subir durante la carga
    fileInputElement.disabled = true;
    uploadButton.disabled = true;
    
    // Muestra la barra de progreso y el botón de cancelar
    progressContainer.style.display = 'block';
    
    let xhr = new XMLHttpRequest();
    let formData = new FormData();

    // Añadir los archivos seleccionados al formData
    for (let i = 0; i < files.length; i++) {
      formData.append('files[]', files[i]);
    }

    // Evento para controlar el progreso de la subida
    xhr.upload.onprogress = function(event) {
      if (event.lengthComputable) {
        const percentComplete = (event.loaded / event.total) * 100;
        progressBar.value = percentComplete;
      }
    };

    // Configuración de la solicitud
    xhr.open('POST', 'upload.php', true);

    // Maneja el evento cuando la subida termina
    xhr.onload = function() {
      if (xhr.status === 200) {
        location.reload();
      } else {
        alert('Error en la subida de archivos');
      }
      // Oculta la barra de progreso y el botón de cancelar tras completar la subida (éxito o fallo)
      progressContainer.style.display = 'none';
      // Habilitar los controles de archivo después de la subida
      fileInputElement.disabled = false;
      uploadButton.disabled = false;
    };

    // Si ocurre un error en la solicitud, cancelar la subida y ocultar todo
    xhr.onerror = function() {
      fileInputElement.disabled = false;
      uploadButton.disabled = false;
      progressBar.value = 0;
      progressContainer.style.display = 'none';
      fileInput.value = ''; // Restablece el campo de archivos seleccionado
      alert('Error en la subida de archivos');
    };

    // Cancelar la subida
    cancelButton.onclick = function() {
      xhr.abort();  // Aborta la solicitud actual
      location.reload();
      progressContainer.style.display = 'none';
      fileInputElement.disabled = false;
      uploadButton.disabled = false;
    };

    // Envía los archivos
    xhr.send(formData);
  });
</script>

</body>
</html>
