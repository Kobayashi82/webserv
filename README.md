# Webserv 1.0

## 🌐 Descripción
**Webserv** es un servidor web programado en C++ con soporte para el protocolo HTTP 1.1. Está diseñado para servir cualquier página web que no requiera SSL/TLS, siempre que esté configurado correctamente.

## ✨ Características
- **Soporte HTTP/1.1**: Webserv acepta y procesa peticiones HTTP según el protocolo estándar.
- **Configuración personalizable**: Se puede iniciar Webserv con archivos de configuración específicos.
- **Soporte para CGI**: Ejecuta scripts CGI (por ejemplo, PHP, Python, etc.).
- **Manejo de logs**: Webserv genera logs tanto de acceso como de errores.
- **Manejo de múltiples direcciones IP y puertos**: Puede configurar varios servidores virtuales con diferentes IP y puertos.
- **Listado de directorios**: Soporta funcionalidad de autoindex.
- **Páginas de error personalizadas**: Páginas de error configurables para diferentes códigos HTTP.
- **Redirecciones**: Soporta redirecciones HTTP.
- **Limitación de solicitudes**: Tamaño máximo de cuerpo configurable.
- **Conexiones keep-alive**: Mantiene conexiones persistentes.

## 🔧 Instalación

```bash
git clone git@github.com:Kobayashi82/Webserv.git
cd webserv
make
```

## 🚀 Opciones de uso
- **Modo gráfico**: `./webserv` abre Webserv en modo gráfico por defecto.
- **Modo consola**: `./webserv -i` inicia Webserv en modo consola.
- **Validación**: `./webserv -t` valida el archivo de configuración.
- **Modo background**: `./webserv &` ejecuta Webserv en segundo plano.
- **Archivo de configuración personalizado**: `./webserv path/to/config.cfg` utiliza un archivo de configuración específico.

Si no se especifica un archivo de configuración, se utiliza el archivo `default.cfg`. Si este no existe, se creará automáticamente.

## ⚙️ Archivo de configuración

El archivo de configuración permite personalizar el comportamiento del servidor. Las configuraciones pueden incluir:

- **Directivas globales**: Aplicables a todos los servidores virtuales.
- **Configuración de servidores virtuales**: Especifica IP, puertos, y host administrado.
- **Configuraciones de ubicación**: Para manejar diferentes rutas en el servidor.
  
### Ejemplo de archivo de configuración:

```config
http {
    body_maxsize 10M;
    cgi .php    /usr/bin/php-cgi;
    cgi .py     cgi-bin/python-cgi;
    cgi .cgi    cgi-bin/test-cgi;
    
    server {
        listen 8081;
        listen 127.0.0.0/24:8085;
        server_name Default
        
        root www/html/;
        index index.php index.html;
        
        location / {
            try_files $uri $uri/ =404;
        }
        
        location /directory/ {
            autoindex on;
        }
        
        error_page 403 /error_pages/403_Forbidden/index.html;
        
        access_log    logs/default_access.log;
        error_log    logs/default_error.log;
    }
}
```

### Directivas principales
| Directiva | Descripción |
|-----------|-------------|
| `body_maxsize` | Tamaño máximo permitido para el cuerpo de las solicitudes |
| `cgi` | Configuración para procesadores CGI por extensión de archivo |
| `listen` | Puerto o IP:Puerto en el que escuchar |
| `server_name` | Nombre del servidor virtual |
| `root` | Directorio raíz para los archivos servidos |
| `index` | Archivos predeterminados a servir cuando se solicita un directorio |
| `location` | Configuración específica para una ruta determinada |
| `try_files` | Intenta servir archivos en un orden específico |
| `autoindex` | Activa/desactiva el listado de directorios |
| `error_page` | Define páginas personalizadas para códigos de error HTTP |
| `access_log` | Ubicación del archivo de log para registrar accesos |
| `error_log` | Ubicación del archivo de log para registrar errores |

## 📁 Estructura del proyecto
El proyecto sigue una organización estructurada:

- **/bin**: Contiene archivos de configuración, ejecutables CGI y contenido web
  - **/default.cfg**: Configuración por defecto
  - **/cgi-bin**: Ejecutables CGI
  - **/www**: Contenido web y recursos
- **/doc**: Archivos de documentación
- **/inc**: Archivos de cabecera organizados por funcionalidad
  - **/Display**: Interfaz de usuario
  - **/Log**: Sistema de logging
  - **/Network**: Gestión de conexiones y sockets
  - **/Protocol**: Implementación del protocolo HTTP
  - **/Settings**: Configuración del servidor
  - **/Thread**: Gestión de hilos
  - **/Utils**: Utilidades comunes
- **/src**: Archivos fuente correspondientes a la organización de cabeceras

## 💻 Implementación técnica
- **Programado en C++98**
- **Sistema multi-hilo**:
  - Hilo principal para gestión de conexiones
  - Hilo secundario para la interfaz de usuario en terminal
  - Hilo dedicado para el sistema de logs
- **Uso de epoll** para manejar múltiples conexiones de manera eficiente
- **Análisis y generación** de solicitudes y respuestas HTTP/1.1
- **Medidas de seguridad** contra solicitudes maliciosas
- **Sistema de caché de archivos** para mejorar el rendimiento
