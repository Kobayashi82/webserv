# webserv

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
