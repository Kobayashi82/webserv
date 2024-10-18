# Webserv 1.0

### Descripción
**Webserv 1.0** es un servidor web programado en C++ con soporte para el protocolo HTTP 1.1. Está diseñado para servir cualquier página web que no requiera SSL/TLS, siempre que esté configurado correctamente.

---

### Características principales
- **Soporte HTTP/1.1**: Webserv acepta y procesa peticiones HTTP según el protocolo estándar.
- **Configuración personalizable**: Se puede iniciar Webserv con archivos de configuración específicos.
- **Soporte para CGI**: Ejecuta scripts CGI (por ejemplo, PHP, Python, etc.).
- **Manejo de logs**: Webserv genera logs tanto de acceso como de errores.
- **Manejo de múltiples direcciones IP y puertos**: Puede configurar varios servidores virtuales con diferentes IP y puertos.
  
---

### Opciones de uso
- **Modo gráfico**: `./webserv` abre Webserv en modo gráfico por defecto.
- **Modo consola**: `./webserv -i` inicia Webserv en modo consola.
- **Validación**: `./webserv -t` valida el archivo de configuración.
- **Modo background**: `./webserv &` ejecuta Webserv en segundo plano.

Si no se especifica un archivo de configuración, se utiliza el archivo `default.cfg` en el directorio del ejecutable. Si no existe, se creará automáticamente.

---

### Archivo de configuración

El archivo de configuración permite personalizar el comportamiento del servidor. Las configuraciones pueden incluir:

- **Directivas globales**: Aplicables a todos los servidores virtuales.
- **Configuración de servidores virtuales**: Especifica IP, puertos, y host administrado.
- **Configuraciones de ubicación**: Para manejar diferentes rutas en el servidor.
  
Ejemplo de archivo de configuración:

```config
http {
    access_log logs/access.log;
    error_log logs/error.log;
    keepalive_timeout 60s;
    root /var/www/;
    index index.php index.html;
    
    server {
        listen 127.0.0.1:8080;
        server_name ejemplo.com;
        location / {
            index index.html;
        }
    }
}
```
