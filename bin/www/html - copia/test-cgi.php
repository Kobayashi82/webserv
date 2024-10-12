<!DOCTYPE html>
<html>
    <head>
        <meta charset="utf-8">
        <title>TEST PHP - Recepción por php://input</title>
        <style>
            body {
                font-family: Arial, sans-serif;
                background-color: #f5f5f5;
                margin: 20px;
            }
            h2 {
                color: #2c3e50;
            }
            .container {
                background-color: #ffffff;
                border: 1px solid #dcdcdc;
                padding: 15px;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
            }
            .data {
                color: #333;
                background-color: #eaf2f8;
                padding: 5px;
                margin: 5px 0;
                border-radius: 5px;
            }
        </style>
    </head>
    <body>
        <div class="container">

            <?php
                echo "<p>Método de solicitud: " . $_SERVER['REQUEST_METHOD'] . "</p>";
            ?>

            <h2>POST (php://input)</h2>
        
            <?php
            // Leer el contenido de php://input
            $bodyContent = file_get_contents('php://input');

            // Si el contenido no está vacío, procesamos los datos
            if (!empty($bodyContent)) {
                // Si el contenido es URL-encoded (como en un formulario POST estándar)
                parse_str($bodyContent, $data);

                if (!empty($data)) {
                    foreach ($data as $key => $value) {
                        echo $key;
                        echo " :";
                        echo htmlspecialchars($value);  // Usar htmlspecialchars para evitar inyecciones
                        echo "<br>";
                    }
                } else {
                    echo "<p>No se recibieron datos válidos.</p>";
                }
            } else {
                echo "<p>No se recibieron datos por php://input.</p>";
            }
            ?>

            <h2>GET</h2>
            
            <?php
            // Verificar si hay datos en $_GET (si el método es GET)
            if (!empty($_GET)) {
                foreach ($_GET as $key => $value) {
                    echo $key;
                    echo " :";
                    echo htmlspecialchars($value);  // Usar htmlspecialchars para evitar inyecciones
                    echo "<br>";
                }
            } else {
                echo "<p>No se recibieron datos por GET.</p>";
            }
            ?>
            
        </div>
    </body>
</html>
