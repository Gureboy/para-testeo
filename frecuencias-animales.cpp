#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h> 

const int audioPin = A0;  // Salida de audio en el pin A0

AsyncWebServer server(80);

// Frecuencias para cada animal/plaga
void activarFrecuencia(int frecuencia) {
  tone(audioPin, frecuencia);
}

void detenerFrecuencia() {
  noTone(audioPin);
}

void setup() {
  Serial.begin(115200);
  pinMode(audioPin, OUTPUT);
  
  // WiFiManager para configurar WiFi
  WiFiManager wifiManager;
  wifiManager.autoConnect("ControlPlagas");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Rutas para cada animal/plaga
  server.on("/mosquitos", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(20000);  // Mosquitos: 20 kHz
    request->send(200, "text/plain", "Frecuencia de mosquitos activada");
  });

  server.on("/palomas", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(25000);  // Palomas: 25 kHz
    request->send(200, "text/plain", "Frecuencia de palomas activada");
  });

  server.on("/polillas", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(30000);  // Polillas: 30 kHz
    request->send(200, "text/plain", "Frecuencia de polillas activada");
  });

  server.on("/gatos", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(18000);  // Gatos: 18 kHz
    request->send(200, "text/plain", "Frecuencia de gatos activada");
  });

  server.on("/perros", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(16000);  // Perros: 16 kHz
    request->send(200, "text/plain", "Frecuencia de perros activada");
  });

  server.on("/moscas", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(22000);  // Moscas: 22 kHz
    request->send(200, "text/plain", "Frecuencia de moscas activada");
  });

  server.on("/cucarachas", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(35000);  // Cucarachas: 35 kHz
    request->send(200, "text/plain", "Frecuencia de cucarachas activada");
  });

  server.on("/murcielagos", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(50000);  // Murciélagos: 50 kHz
    request->send(200, "text/plain", "Frecuencia de murciélagos activada");
  });

  server.on("/ratones", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(45000);  // Ratones: 45 kHz
    request->send(200, "text/plain", "Frecuencia de ratones activada");
  });

  server.on("/pulgas", HTTP_GET, [](AsyncWebServerRequest *request){
    activarFrecuencia(40000);  // Pulgas: 40 kHz
    request->send(200, "text/plain", "Frecuencia de pulgas activada");
  });

  server.begin();
}

void loop() {
  unsigned long tiempoInicio = 0;
unsigned long duracion = 60000; // 1 minuto

void loop() {
  if (millis() - tiempoInicio > duracion) {
    detenerFrecuencia();
  }
}

}

// HTML para la página web
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Control de Plagas - Saluduino</title>
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
    .slider {
      appearance: none;
      width: 60px;
      height: 34px;
      background: #ccc;
      border-radius: 34px;
      position: relative;
      outline: none;
      transition: background 0.2s;
    }
    .slider:before {
      content: '';
      position: absolute;
      width: 26px;
      height: 26px;
      border-radius: 50%;
      background: white;
      transition: transform 0.2s;
    }
    .slider:checked {
      background: #4caf50;
    }
    .slider:checked:before {
      transform: translateX(26px);
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
