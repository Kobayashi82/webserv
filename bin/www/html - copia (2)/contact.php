<?php
// Verificar si el formulario fue enviado
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    // Leer los datos del cuerpo de la solicitud
    parse_str(file_get_contents("php://input"), $postData);

    // Obtener los datos del formulario
    $name = isset($postData['name']) ? trim($postData['name']) : '';
    $email = isset($postData['email']) ? trim($postData['email']) : '';
    $subject = isset($postData['subject']) ? trim($postData['subject']) : '';
    $message = isset($postData['message']) ? trim($postData['message']) : '';

    // Verificar si los campos requeridos no están vacíos
    if (!empty($name) && !empty($email) && !empty($subject) && !empty($message)) {
        // Validar el formato del email
        if (!filter_var($email, FILTER_VALIDATE_EMAIL)) {
            echo json_encode(['success' => false, 'message' => 'Invalid email format']);
            exit();
        }

        // Verificar si la carpeta "messages" existe, y si no, crearla
        $directory = 'messages';
        if (!is_dir($directory)) {
            mkdir($directory, 0777, true);  // Crear la carpeta con permisos 0777
        }

        // Guardar el mensaje en un archivo dentro de la carpeta "messages"
        $filePath = 'messages/' . uniqid('message_') . '.txt';
        $messageContent = "Name: $name\nEmail: $email\nSubject: $subject\nMessage: $message\n\n";
        file_put_contents($filePath, $messageContent);

        // Responder con éxito
        echo json_encode(['success' => true, 'message' => 'Message sent successfully!']);

        exit();
    } else {
        echo json_encode(['success' => false, 'message' => 'Please fill in all the fields']);
        exit();
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebServ 1.0 - Contact Us</title>
    <link rel="stylesheet" href="./style.css">
</head>
<body>
<div class="signup">
    <h1>Contact Us</h1>
    <form id="contactForm" method="post" novalidate>
        <input type="text" name="name" id="name" placeholder="Your Name" required="required" />
        <input type="email" name="email" id="email" placeholder="Your Email" required="required" />
        <input type="text" name="subject" id="subject" placeholder="Subject" required="required" />
        <textarea name="message" id="message" placeholder="Your Message" required="required" rows="5"></textarea>
        <button type="submit" class="btn btn-primary btn-block btn-large">Send Message</button>
    </form>
    <p style="text-align: center; margin-top: 15px; font-size: 12px; color: gray;">
      Need assistance? <a href="/support.html" style="color: yellow; margin-left: 5px;">Get Support</a>
    </p>
    <p id="response-message"></p>
</div>

<script>
document.getElementById('contactForm').addEventListener('submit', function(e) {
    e.preventDefault(); // Prevenir el comportamiento por defecto del formulario
    
    // Validaciones personalizadas
    const name = document.getElementById('name');
    const email = document.getElementById('email');
    const subject = document.getElementById('subject');
    const message = document.getElementById('message');
    
    let formIsValid = true; // Variable para saber si el formulario es válido

    // Validar nombre
    if (name.value === '') {
        name.setCustomValidity('Please enter your name');
        formIsValid = false;
    } else {
        name.setCustomValidity('');
    }

    // Validar email
    const emailRegex = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/;
    if (email.value === '') {
        email.setCustomValidity('Please enter your email');
        formIsValid = false;
    } else if (!emailRegex.test(email.value)) {
        email.setCustomValidity('Please enter a valid email');
        formIsValid = false;
    } else {
        email.setCustomValidity('');
    }

    // Validar asunto
    if (subject.value === '') {
        subject.setCustomValidity('Please enter the subject');
        formIsValid = false;
    } else {
        subject.setCustomValidity('');
    }

    // Validar mensaje
    if (message.value === '') {
        message.setCustomValidity('Please enter your message');
        formIsValid = false;
    } else {
        message.setCustomValidity('');
    }

    // Si algún campo no es válido, no enviamos el formulario
    if (!formIsValid) {
        document.getElementById('contactForm').reportValidity();
        return; // Salir del submit
    }

    // Si la validación es correcta, enviamos el formulario usando Fetch API
    const formData = `name=${encodeURIComponent(name.value)}&email=${encodeURIComponent(email.value)}&subject=${encodeURIComponent(subject.value)}&message=${encodeURIComponent(message.value)}`;

    fetch('/contact.php', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: formData
    })
    .then(response => response.json())
    .then(data => {
        const responseMessage = document.getElementById('response-message');
        if (data.success) {
            alert('Message sent successfully!');
        } else {
            alert('Message sent failed!');
        }
    })
    .catch(error => {
        console.error('Error:', error);
        alert('There was an error processing your request');
    });
    name.value = '';
    email.value = '';
    subject.value = '';
    message.value = '';
});
</script>
</body>
</html>
