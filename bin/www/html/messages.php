<?php
$output = shell_exec('python3 messages.py'); // Ejecutar el script de Python
$files = json_decode($output, true); // Decodificar la salida JSON del script de Python
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8"> <!-- Define el tipo de caracteres utilizado en la página -->
    <meta name="viewport" content="width=device-width, initial-scale=1.0"> <!-- Configura el viewport para adaptabilidad en móviles -->
    <title>Mensajes</title> <!-- Título de la página -->
    <link rel="stylesheet" href="resources/style.css"> <!-- Enlace a la hoja de estilos -->
</head>
<body class="messages-page">
<div class="messages-container">
    <h1>Mensajes</h1>

    <?php if (empty($files)): ?>
        <div class="message-box">
            <p>No hay ningún mensaje</p>
        </div>
    <?php else: ?>
        <?php foreach ($files as $content): ?>
            <div class="message-box">
                <?php
                $lines = explode("\n", $content);
                foreach ($lines as $line):
                ?>
                    <p><?php echo htmlspecialchars($line); ?></p>
                <?php endforeach; ?>
            </div>
        <?php endforeach; ?>
    <?php endif; ?>
</div>
</body>
</html>