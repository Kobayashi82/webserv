<?php
// Establecer la cabecera de tipo MIME para HTML
header('Content-Type: text/html; charset=UTF-8');
?>

<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Mostrar contenido del cuerpo recibido</title>
</head>
<body>
    <h1>Contenido del cuerpo recibido</h1>
    <pre>
        <?php
        if ($_SERVER['REQUEST_METHOD'] === 'POST') {
            // Leer el contenido del cuerpo de la solicitud
            $bodyContent = file_get_contents('php://input');

            // Mostrar el contenido del cuerpo
            if (!empty($bodyContent)) {
                echo "Cuerpo recibido desde php://input:\n";
                echo htmlspecialchars($bodyContent);  // Usar htmlspecialchars para evitar inyección de HTML
            } else {
                echo "No se recibió ningún dato en el cuerpo del mensaje.\n";
            }
        } else {
            echo "Esta página solo recibe solicitudes POST.";
        }
        ?>
    </pre>
</body>
</html>
