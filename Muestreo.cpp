#include <MAX30100.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
##include <ESP8266HTTPClient.h>
#include <Base64.h> // Para codificación Base64

// Configuración de la red
const char* ssid = "tu_SSID";
const char* password = "tu_CONTRASEÑA";

// Autenticación
const char* usuarioAdmin = "admin";
const char* claveAdmin = "adminpass";

// Configuración de alertas
const float maxRitmoCardiaco = 120.0;
const float minRitmoCardiaco = 50.0;
const float maxSpO2 = 98.0; // Ajustar para saturación de oxígeno en sangre
const float minSpO2 = 90.0; // Ajustar para saturación de oxígeno en sangre

// Estructura para almacenar pacientes
struct Paciente {
  String nombre;
  float ritmoCardiaco;
  float spO2; // Usaremos SpO2 en lugar de temperatura
};

// Lista de pacientes
Paciente pacientes[10];
int cantidadPacientes = 0;

// Datos del último usuario
String ultimoUsuario = "Ninguno";

// Variables para el MAX30105
MAX30105 pox;
Ticker ticker;
#define REPORTING_PERIOD_MS 1000
uint32_t tsLastReport = 0;

// Servidor web
ESP8266WebServer server(80);

// Página HTML con un lugar para mostrar la IP
const char* paginaHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Saluduino - Sistema de Monitoreo</title>
  <style>
    /* ... estilos existentes ... */
  </style>
</head>
<body>
  <div class="header">
    <h1>Bienvenido a Saluduino</h1>
    <p>¿Qué desea hacer?</p>
    <button class="button" onclick="location.href='/agregar'">Agregar Paciente</button>
    <button class="button" onclick="location.href='/ver'">Observar Lista de Pacientes</button>
    <button class="button" onclick="location.href='/eliminar'">Eliminar Paciente</button>
  </div>
  <div class="container">
    <h2>Tu dirección IP: <span id="ip_address"></span></h2>
  </div>
  <script>
    // Obtener la IP cuando se carga la página y mostrarla
    window.onload = function() {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'https://api.ipify.org?format=text', true);
      xhr.onload = function() {
        if (xhr.status === 200) {
          document.getElementById('ip_address').textContent = xhr.responseText;
        }
      };
      xhr.send();
    };
  </script>
</body>
</html>
)rawliteral";

// ... funciones restantes para manejar el envío de formularios, gestión de pacientes, etc. ...

// Configuración inicial
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi");

  pox.begin(); // Inicializar sensor MAX30105

  ticker.attach_ms(REPORTING_PERIOD_MS, []() {
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      tsLastReport = millis();
    }
  });

  configurarRutas();
  server.begin();
  Serial.println("Servidor iniciado");

  // Obtener y mostrar la IP en el monitor serial
  IPAddress ip = WiFi.localIP();
  Serial.print("Tu dirección IP local: ");
  Serial.println(ip);
}

// ... bucle principal para manejar el servidor web y los datos del sensor ...
