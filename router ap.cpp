#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Configuración del punto de acceso
const char* ssid = "ESP8266_Router";      // Nombre de la red WiFi (SSID)
const char* password = "12345678";         // Contraseña de la red (mínimo 8 caracteres)
IPAddress local_IP(192, 168, 4, 1);        // IP fija del AP
IPAddress gateway(192, 168, 4, 1);         // Puerta de enlace
IPAddress subnet(255, 255, 255, 0);        // Máscara de subred

// Configuración del servidor web en el puerto 80
ESP8266WebServer server(80);

void setup() {
  // Inicializamos el serial para monitorización
  Serial.begin(115200);
  delay(100);

  // Configuramos la IP fija del AP
  WiFi.softAPConfig(local_IP, gateway, subnet);

  // Configuramos el ESP8266 como AP
  WiFi.softAP(ssid, password);

  // Mostramos la IP del AP en el serial
  Serial.println("Punto de acceso configurado con IP fija.");
  Serial.print("IP del AP: ");
  Serial.println(WiFi.softAPIP());

  // Configuramos la ruta "/" para la página de inicio
  server.on("/", handleRoot);

  // Iniciamos el servidor web
  server.begin();
  Serial.println("Servidor web iniciado.");
}

void handleRoot() {
  // Contenido HTML con estilo AdminLTE
  String html = "<!DOCTYPE html><html lang='es'><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Inicio - ESP8266 Router</title>";

  // Carga CSS de AdminLTE desde un CDN
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css'>";
  html += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/admin-lte@3.1/dist/css/adminlte.min.css'>";
  html += "</head><body class='hold-transition sidebar-mini layout-fixed'>";

  // Encabezado
  html += "<div class='wrapper'>";
  html += "<nav class='main-header navbar navbar-expand navbar-white navbar-light'>";
  html += "<a href='#' class='navbar-brand'><span class='brand-text font-weight-light'>ESP8266 Router</span></a>";
  html += "</nav>";

  // Sidebar
  html += "<aside class='main-sidebar sidebar-dark-primary elevation-4'>";
  html += "<a href='#' class='brand-link'><span class='brand-text font-weight-light'>AdminLTE</span></a>";
  html += "<div class='sidebar'><nav class='mt-2'><ul class='nav nav-pills nav-sidebar flex-column' role='menu'>";
  html += "<li class='nav-item'><a href='#' class='nav-link active'><i class='nav-icon fas fa-home'></i><p>Inicio</p></a></li>";
  html += "<li class='nav-item'><a href='/config' class='nav-link'><i class='nav-icon fas fa-cogs'></i><p>Configuración</p></a></li>";
  html += "<li class='nav-item'><a href='/status' class='nav-link'><i class='nav-icon fas fa-info-circle'></i><p>Estado</p></a></li>";
  html += "</ul></nav></div></aside>";

  // Contenido principal
  html += "<div class='content-wrapper'><section class='content'><div class='container-fluid'>";
  html += "<div class='row'><div class='col-md-12'><div class='card'>";
  html += "<div class='card-header'><h3 class='card-title'>Configuración del Router</h3></div>";
  html += "<div class='card-body'><p>Bienvenido al panel de configuración del ESP8266.</p>";
  html += "<p>IP del dispositivo: " + WiFi.softAPIP().toString() + "</p>";
  html += "<p><a href='/config' class='btn btn-primary'>Configuración avanzada</a></p>";
  html += "</div></div></div></div></div></section></div>";

  // Scripts de AdminLTE y FontAwesome
  html += "<script src='https://cdn.jsdelivr.net/npm/admin-lte@3.1/dist/js/adminlte.min.js'></script>";
  html += "</body></html>";

  // Enviamos el contenido HTML de la página de inicio
  server.send(200, "text/html", html);
}

void loop() {
  // Mantenemos el servidor en funcionamiento
  server.handleClient();
}
