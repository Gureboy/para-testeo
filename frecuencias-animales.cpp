#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

const int audioPin = 13;  // Pin para el tono
ESP8266WebServer server(80);
Ticker frecuenciaTicker;

bool frecuenciaActiva = false;
int frecuenciaActual = 0;

// Frecuencias para cada animal/plaga
void activarFrecuencia(int frecuencia) {
    frecuenciaActual = frecuencia;
    tone(audioPin, frecuencia);
    frecuenciaActiva = true;  // Marca que hay una frecuencia activa
}

void detenerFrecuencia() {
    noTone(audioPin);
    frecuenciaActiva = false;  // Marca que no hay frecuencias activas
}

// Función para alternar encendido/apagado cada 8.3 segundos
void manejarFrecuencia() {
    if (frecuenciaActiva) {
        detenerFrecuencia();  // Apaga la frecuencia
    } else {
        activarFrecuencia(frecuenciaActual);  // Enciende la frecuencia
    }
}

// HTML para la página web
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
      background-color: #f4f6f9;
    }
    .btn-container {
      display: flex;
      flex-wrap: wrap;
      justify-content: center;
    }
    .btn {
      margin: 10px;
      padding: 20px;
      font-size: 20px;
      border-radius: 10px;
      cursor: pointer;
      color: white;
      background-color: #007bff;
      transition: background-color 0.3s;
    }
    .btn:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <div class="content-wrapper">
      <div class="content">
        <div class="container-fluid">
          <h1 class="m-0 text-dark text-center">Control de Plagas y Animales</h1>
          <div class="btn-container">
            <div>
              <label class="btn">🦟 Mosquitos
                <input type="checkbox" class="slider" onchange="toggleFrequency('/mosquitos', this)">
              </label>
            </div>
            <div>
              <label class="btn">🕊️ Palomas
                <input type="checkbox" class="slider" onchange="toggleFrequency('/palomas', this)">
              </label>
            </div>
            <div>
              <label class="btn">🦋 Polillas
                <input type="checkbox" class="slider" onchange="toggleFrequency('/polillas', this)">
              </label>
            </div>
            <div>
              <label class="btn">🐱 Gatos
                <input type="checkbox" class="slider" onchange="toggleFrequency('/gatos', this)">
              </label>
            </div>
            <div>
              <label class="btn">🐶 Perros
                <input type="checkbox" class="slider" onchange="toggleFrequency('/perros', this)">
              </label>
            </div>
            <div>
              <label class="btn">🪰 Moscas
                <input type="checkbox" class="slider" onchange="toggleFrequency('/moscas', this)">
              </label>
            </div>
            <div>
              <label class="btn">🪳 Cucarachas
                <input type="checkbox" class="slider" onchange="toggleFrequency('/cucarachas', this)">
              </label>
            </div>
            <div>
              <label class="btn">🦇 Murciélagos
                <input type="checkbox" class="slider" onchange="toggleFrequency('/murcielagos', this)">
              </label>
            </div>
            <div>
              <label class="btn">🐭 Ratones
                <input type="checkbox" class="slider" onchange="toggleFrequency('/ratones', this)">
              </label>
            </div>
            <div>
              <label class="btn">🦗 Pulgas
                <input type="checkbox" class="slider" onchange="toggleFrequency('/pulgas', this)">
              </label>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
  <script>
    function toggleFrequency(url, checkbox) {
      const checkboxes = document.querySelectorAll('.slider');
      checkboxes.forEach(cb => {
        if (cb !== checkbox) {
          cb.checked = false;
        }
      });

      fetch(url)
        .then(response => response.text())
        .then(data => console.log(data))
        .catch(error => console.error('Error:', error));
    }
  </script>
</body>
</html>
)rawliteral";

// Función para abrir la página web en la conexión WiFi
void openWebPageCallback() {
    Serial.println("Abriendo página web...");
    // Solo esperamos un momento para la estabilidad
    delay(1000);
    // Aquí podrías usar otro método si quieres abrir la página web automáticamente
}

// Configuración del servidor web
void setupServer() {
    server.on("/", []() {
        server.send_P(200, "text/html", index_html);
    });

    // Rutas para cada animal/plaga
    server.on("/mosquitos", []() {
        activarFrecuencia(10000);  // Mosquitos: 10 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de mosquitos activada");
    });

    server.on("/palomas", []() {
        activarFrecuencia(8000);  // Palomas: 8 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de palomas activada");
    });

    server.on("/polillas", []() {
        activarFrecuencia(9000);  // Polillas: 9 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de polillas activada");
    });

    server.on("/gatos", []() {
        activarFrecuencia(7000);  // Gatos: 7 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de gatos activada");
    });

    server.on("/perros", []() {
        activarFrecuencia(6000);  // Perros: 6 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de perros activada");
    });

    server.on("/moscas", []() {
        activarFrecuencia(8500);  // Moscas: 8.5 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de moscas activada");
    });

    server.on("/cucarachas", []() {
        activarFrecuencia(9500);  // Cucarachas: 9.5 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de cucarachas activada");
    });

    server.on("/murcielagos", []() {
        activarFrecuencia(11000);  // Murciélagos: 11 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de murciélagos activada");
    });

    server.on("/ratones", []() {
        activarFrecuencia(12000);  // Ratones: 12 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de ratones activada");
    });

    server.on("/pulgas", []() {
        activarFrecuencia(13000);  // Pulgas: 13 kHz
        frecuenciaTicker.attach(8.3, manejarFrecuencia);  // Alternar cada 8.3 segundos
        server.send(200, "text/plain", "Frecuencia de pulgas activada");
    });

    server.begin();
    Serial.println("Servidor web iniciado en la IP: " + WiFi.localIP().toString());
}

void setup() {
    Serial.begin(115200);

    // Conectar a la red WiFi
    WiFiManager wifiManager;
    wifiManager.autoConnect("ControldePlaga");

    // Configura la IP fija
    WiFi.softAPConfig(IPAddress(192, 168, 10, 164), IPAddress(192, 168, 10, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("ControldePlaga", "");

    openWebPageCallback();  // Llama a la función de apertura de página
    setupServer();  // Configura el servidor
}

void loop() {
    server.handleClient();  // Maneja las peticiones del cliente
}