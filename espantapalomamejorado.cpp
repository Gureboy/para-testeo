#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

const int audioPin = 13;  // Pin para el tono
const int gpioEstado = 4; // Pin GPIO para el indicador de estado

ESP8266WebServer server(80);
Ticker frecuenciaTicker;

bool frecuenciaActiva = false;
int frecuenciaActual = 0;

// Actualizar estado del GPIO
void actualizarEstadoGPIO() {
    digitalWrite(gpioEstado, frecuenciaActiva ? LOW : HIGH);
}

// Activar frecuencia específica
void activarFrecuencia(int frecuencia) {
    frecuenciaActual = frecuencia;
    tone(audioPin, frecuencia);
    frecuenciaActiva = true;
    actualizarEstadoGPIO();
}

// Detener frecuencia activa
void detenerFrecuencia() {
    noTone(audioPin);
    frecuenciaActiva = false;
    actualizarEstadoGPIO();
}

// Alternar encendido/apagado cada 8.3 segundos
void manejarFrecuencia() {
    if (frecuenciaActiva) {
        detenerFrecuencia();
    } else {
        activarFrecuencia(frecuenciaActual);
    }
}

// HTML con botón de apagado total
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Control de Plagas</title>
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    body {
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background-color: #2f2f2f;
      color: #f8f9fa;
      font-family: 'Source Sans Pro', sans-serif;
      margin: 0;
    }
    .card {
      background: #3d3d3d;
      border-radius: 15px;
      padding: 40px;
      width: 400px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
      text-align: center;
    }
    h1 {
      font-size: 2rem;
      margin-bottom: 30px;
      color: #e1e1e1;
      text-shadow: 1px 1px 3px rgba(0, 0, 0, 0.7);
    }
    .toggle-container {
      display: flex;
      flex-direction: column;
      gap: 20px;
      margin-top: 20px;
    }
    .toggle-group {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 30px;
    }
    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    .slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #6c757d;
      transition: .4s;
      border-radius: 30px;
    }
    .slider:before {
      position: absolute;
      content: "";
      height: 22px;
      width: 22px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }
    input:checked + .slider {
      background-color: #20c997;
    }
    input:checked + .slider:before {
      transform: translateX(28px);
    }
    .shutdown-button {
      margin-top: 30px;
      padding: 10px 20px;
      background-color: #dc3545;
      color: white;
      border: none;
      border-radius: 5px;
      font-size: 1rem;
      cursor: pointer;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.2);
    }
    .shutdown-button:hover {
      background-color: #c82333;
    }
    .footer {
      margin-top: 30px;
      font-size: 0.85rem;
      color: #adb5bd;
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>Control de Plagas 🦟</h1>
    <div class="toggle-container">
      <div class="toggle-group">
        🕊️ Palomas
        <label class="toggle-switch">
          <input type="checkbox" id="palomasToggle" onchange="toggleExclusive('palomasToggle', '/palomas')">
          <span class="slider"></span>
        </label>
      </div>
      <div class="toggle-group">
        🎵 Test
        <label class="toggle-switch">
          <input type="checkbox" id="testToggle" onchange="toggleExclusive('testToggle', '/test')">
          <span class="slider"></span>
        </label>
      </div>
    </div>
    <button class="shutdown-button" onclick="apagarSistema()">⏹️ Apagar Sistema</button>
    <div class="footer">
      <i class="fas fa-copyright"></i> 2024 Control de Plagas
    </div>
  </div>
  <script>
    function toggleExclusive(activeToggle, url) {
      const palomasToggle = document.getElementById('palomasToggle');
      const testToggle = document.getElementById('testToggle');
      if (activeToggle === 'palomasToggle' && testToggle.checked) {
        testToggle.checked = false;
      } else if (activeToggle === 'testToggle' && palomasToggle.checked) {
        palomasToggle.checked = false;
      }
      fetch(url)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }
    function apagarSistema() {
      document.getElementById('palomasToggle').checked = false;
      document.getElementById('testToggle').checked = false;
      fetch('/shutdown')
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }
  </script>
</body>
</html>
)rawliteral";

// Configuración del servidor web
void setupServer() {
    server.on("/", []() {
        server.send_P(200, "text/html", index_html);
    });

    server.on("/palomas", []() {
        detenerFrecuencia();
        activarFrecuencia(18000); // Frecuencia para Palomas: 18 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);
        server.send(200, "text/plain", "Frecuencia de palomas activada");
    });

    server.on("/test", []() {
        detenerFrecuencia();
        activarFrecuencia(8000); // Frecuencia de Test: 15 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);
        server.send(200, "text/plain", "Frecuencia de prueba activada");
    });

    server.on("/shutdown", []() {
        detenerFrecuencia();
        frecuenciaTicker.detach();
        server.send(200, "text/plain", "Sistema apagado");
    });

    server.begin();
    Serial.println("Servidor web iniciado en la IP: " + WiFi.localIP().toString());
}

void setup() {
    Serial.begin(115200);
    pinMode(gpioEstado, OUTPUT);
    actualizarEstadoGPIO();
    WiFiManager wifiManager;
    wifiManager.autoConnect("ControlDePlaga");
    setupServer();
}

void loop() {
    server.handleClient();
}
