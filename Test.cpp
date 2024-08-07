#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h" // Use MAX30100_PulseOximeter.h for PulseOximeter object

// Configuración de red
const char* ssid = "tu_SSID";
const char* password = "tu_CONTRASEÑA";

// Autenticación
const char* usuarioAdmin = "admin";
const char* claveAdmin = "adminpass";

// Configuración de alertas
const float maxRitmoCardiaco = 120.0;
const float minRitmoCardiaco = 50.0;
const float maxTempCorporal = 37.5; // Consider using SpO2 instead of temperature
const float minTempCorporal = 36.0; // Consider using SpO2 instead of temperature

// Estructura para almacenar pacientes
struct Paciente {
  String nombre;
  float ritmoCardiaco;
  float tempCorporal; // Consider using SpO2 here
};

// Lista de pacientes
Paciente pacientes[10];
int cantidadPacientes = 0;

// Datos del último usuario
String ultimoUsuario = "Ninguno";

// Variables para el MAX30100
PulseOximeter pox; // Use PulseOximeter object
Ticker ticker;
#define REPORTING_PERIOD_MS 1000
uint32_t tsLastReport = 0;

// Servidor web
ESP8266WebServer server(80);

// Página HTML con un lugar para mostrar la IP (placeholder)
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
    <p>Tu dirección IP local: <span id="ip_address"></span></p>
    <p>¿Qué desea hacer?</p>
    <button class="button" onclick="location.href='/agregar'">Agregar Paciente</button>
    <button class="button" onclick="location.href='/ver'">Observar Lista de Pacientes</button>
    <button class="button" onclick="location.href='/eliminar'">Eliminar Paciente</button>
  </div>
  <div class="container">
    </div>
  <script>
    // Obtener la IP local y mostrarla en el elemento con id "ip_address"
    window.onload = function() {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', 'http://192.168.1.1/status', true); // Replace with your router's IP if needed
      xhr.onload = function() {
        if (xhr.status === 200) {
          var respuesta = xhr.responseText;
          var ipRegex = /IP Address: (\d+\.\d+\.\d+\.\d+)/;
          var match = ipRegex.exec(respuesta);
          if (match) {
            document.getElementById('ip_address').textContent = match[1];
          }
        }
      };
      xhr.send();
    };
  </script>
</body>
</html>
)rawliteral";

// ... funciones restantes para manejar el sensor, formulario, etc. ...

// Configuración inicial
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi");

  pox.begin(); // Inicializar sensor MAX30100
  ticker.attach_ms(REPORTING_PERIOD_MS, []() {
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
      tsLastReport = millis();
    }
  });

  configurarRutas();
  server.begin();
  Serial.println
