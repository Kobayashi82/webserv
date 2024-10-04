$(document).ready(function() {
   $('.contact-form').on('submit', function(event) {
     event.preventDefault(); // Prevenir el envío normal del formulario
 
     // Crear un nuevo formulario
     var form = $('<form>', {
       'action': '/samples/index.cgi',
       'method': 'POST'
     });
 
     // Añadir los campos del formulario original al nuevo formulario
     form.append($('<input>', {
       'name': 'name',
       'value': $('#name').val(),
       'type': 'hidden'
     }));
     
     form.append($('<input>', {
       'name': 'email',
       'value': $('#email').val(),
       'type': 'hidden'
     }));
     
     form.append($('<input>', {
       'name': 'message',
       'value': $('#message').val(),
       'type': 'hidden'
     }));
 
     // Añadir el formulario al `body` (es invisible para el usuario)
     $('body').append(form);
 
     // Enviar el formulario
     form.submit();
   });
 
   // Manejo de clase not-empty
   $('.js-input').keyup(function() {
     if ($(this).val()) {
       $(this).addClass('not-empty');
     } else {
       $(this).removeClass('not-empty');
     }
   });
 });
 
 