<?php
// Establecer la cabecera para indicar que el contenido es texto plano
header('Content-Type: text/plain; charset=UTF-8');

// Leer el contenido del cuerpo de la solicitud
$bodyContent = file_get_contents('php://input');

// Imprimir el contenido del cuerpo, independientemente del formato
if (!empty($bodyContent)) {
    echo "Cuerpo recibido:\n\n";
    echo $bodyContent;
} else {
    echo "No se recibió ningún dato en el cuerpo del mensaje.";
}