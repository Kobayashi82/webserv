<?php
session_start();

// Función para verificar si las credenciales almacenadas en la cookie son válidas
function checkUserSession($username) {
    // Intentar abrir el archivo de usuarios
    $userdata = @file_get_contents('userdata');
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

// Función para formatear el tamaño del archivo en KB, MB, GB, etc.
function formatFileSize($size) {
    $unit = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB'];
    $mod = 1024;
    $i = 0;

    while ($size >= $mod && $i < count($unit) - 1) {
        $size /= $mod;
        $i++;
    }

    // Usamos floatval() para asegurar que el valor de $size es un número
    return round(floatval($size), 2) . ' ' . $unit[$i];
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

// Verifica si el usuario está logueado (por sesión o cookie)
if (!isset($_SESSION['user_session'])) {
    header('Location: login.php');
    exit();
}

// Obtiene el nombre de usuario y convierte a minúsculas
$username = strtolower($_SESSION['user_session']);

// Define el directorio del usuario en minúsculas
$userDirectory = 'users/' . $username;

// Verifica si la carpeta del usuario existe, si no, crea la carpeta
if (!is_dir($userDirectory)) {
    if (!mkdir($userDirectory, 0777, true)) {
        echo "Error al crear la carpeta del usuario.";
        exit();
    }
}

// Lee los archivos dentro de la carpeta del usuario
$files = scandir($userDirectory);
$filesList = [];
foreach ($files as $file) {
    // Ignora las carpetas '.' y '..'
    if ($file == '.' || $file == '..') {
        continue;
    }

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
  <!-- Enlace a Font Awesome para usar iconos -->
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <link rel="stylesheet" href="style.css">
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
    <img src="banner.jpg" alt="Banner" class="banner">
    <a href="logout.php" class="logout-link">Logout</a>
  </header>

  <!-- Separador bonito debajo del banner -->
  <hr class="separator">

  <!-- Cuadro para la lista de archivos -->
  <div class="file-box">
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
              <!-- Sustituir los botones por iconos -->
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
            // Recarga la página después de eliminar el archivo
            location.reload();  // Recarga la página para reflejar los cambios
        } else {
            // Muestra el mensaje de error solo si la eliminación falla
            alert('Error al eliminar el archivo: ' + response.message);
            location.reload();
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
    };

    // Cancelar la subida
    cancelButton.onclick = function() {
      xhr.abort();  // Aborta la solicitud actual
      alert('Subida cancelada');
      progressContainer.style.display = 'none';
      location.reload();
    };

    // Envía los archivos
    xhr.send(formData);
  });
  </script>
</body>
</html>
