#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

const int audioPin = 13; // Pin para el tono
ESP8266WebServer server(80);
Ticker frecuenciaTicker;

bool frecuenciaActiva = false;
int frecuenciaActual = 0;

// Frecuencia para Palomas
void activarFrecuencia(int frecuencia) {
    frecuenciaActual = frecuencia;
    tone(audioPin, frecuencia);
    frecuenciaActiva = true;
}

void detenerFrecuencia() {
    noTone(audioPin);
    frecuenciaActiva = false;
}

// Alternar encendido/apagado cada 8.3 segundos
void manejarFrecuencia() {
    if (frecuenciaActiva) {
        detenerFrecuencia();
    } else {
        activarFrecuencia(frecuenciaActual);
    }
}

// HTML actualizado con checkbox y botón de apagado
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
      flex-direction: column;
      justify-content: center;
      align-items: center;
      height: 100vh;
      background-color: #f4f6f9;
    }
    .btn-container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
      gap: 20px;
    }
    .btn {
      margin: 10px;
      padding: 15px;
      font-size: 18px;
      border-radius: 8px;
      color: white;
      background-color: #007bff;
      cursor: pointer;
      transition: background-color 0.3s;
    }
    .btn:hover {
      background-color: #0056b3;
    }
    .btn-apagar {
      background-color: #dc3545;
    }
    .btn-apagar:hover {
      background-color: #c82333;
    }
    .switch-label {
      display: flex;
      align-items: center;
      cursor: pointer;
      font-size: 20px;
      margin-bottom: 20px;
    }
  </style>
</head>
<body>
  <h1>Control de Plagas</h1>
  <div class="btn-container">
    <label class="switch-label">
      🕊️ Palomas
      <input type="checkbox" id="palomasCheckbox" onchange="toggleFrequency('/palomas', this)">
    </label>
    <button class="btn btn-apagar" onclick="apagarFrecuencia()">🔴 Apagar</button>
  </div>
  <script>
    function toggleFrequency(url, checkbox) {
      fetch(url)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }

    function apagarFrecuencia() {
      const checkbox = document.getElementById('palomasCheckbox');
      if (checkbox.checked) checkbox.checked = false;
      fetch('/apagar')
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
        activarFrecuencia(8000); // Frecuencia para Palomas: 8 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);
        server.send(200, "text/plain", "Frecuencia de palomas activada");
    });

    server.on("/apagar", []() {
        detenerFrecuencia();
        frecuenciaTicker.detach(); // Detiene el ticker
        server.send(200, "text/plain", "Frecuencia desactivada");
    });

    server.begin();
    Serial.println("Servidor web iniciado en la IP: " + WiFi.localIP().toString());
}

void setup() {
    Serial.begin(115200);

    // Conectar a la red WiFi
    WiFiManager wifiManager;
    wifiManager.autoConnect("ControldePlaga");

    setupServer();
}

void loop() {
    server.handleClient();
}
