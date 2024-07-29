#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Configuración de red
const char* ssid = "tu_SSID";
const char* password = "tu_CONTRASEÑA";

// Autenticación
const char* usuarioAdmin = "admin";
const char* claveAdmin = "adminpass";

// Configuración de alertas
const float maxRitmoCardiaco = 120.0;
const float minRitmoCardiaco = 50.0;
const float maxTempCorporal = 37.5;
const float minTempCorporal = 36.0;

// Estructura para almacenar pacientes
struct Paciente {
    String nombre;
    float ritmoCardiaco;
    float tempCorporal;
};

// Lista de pacientes
Paciente pacientes[10];
int cantidadPacientes = 0;

// Datos del último usuario
String ultimoUsuario = "Ninguno";

// Variables para el MAX30100
PulseOximeter pox;
Ticker ticker;
#define REPORTING_PERIOD_MS 1000
uint32_t tsLastReport = 0;

// Servidor web
ESP8266WebServer server(80);

// Página HTML
const char* paginaHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Saluduino - Sistema de Monitoreo</title>
  <style>
    body { font-family: Arial, sans-serif; background-color: #f4f4f9; margin: 0; }
    .container { padding: 20px; }
    .header { background-color: #4CAF50; color: white; padding: 10px; text-align: center; }
    .button { padding: 10px 20px; margin: 10px; border: none; border-radius: 5px; color: white; background-color: #4CAF50; cursor: pointer; }
    .button:hover { background-color: #45a049; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #f2f2f2; }
    .alerta { color: red; font-weight: bold; }
    canvas { max-width: 100%; height: auto; }
    .theme-toggle { margin-top: 20px; }
    .theme-light body { background-color: #ffffff; color: #000000; }
    .theme-dark body { background-color: #333333; color: #ffffff; }
    .formulario { margin-top: 20px; }
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
    <h2>Formulario de Datos del Paciente</h2>
    <form action="/enviar" method="post" class="formulario">
      <label for="nombre">Nombre:</label>
      <input type="text" id="nombre" name="nombre" required>
      <label for="ritmoCardiaco">Ritmo Cardiaco (bpm):</label>
      <input type="number" id="ritmoCardiaco" name="ritmoCardiaco" step="0.1" required>
      <label for="tempCorporal">Temperatura Corporal (°C):</label>
      <input type="number" id="tempCorporal" name="tempCorporal" step="0.1" required>
      <input type="submit" value="Enviar" class="button">
    </form>
    <h2>Pacientes Almacenados</h2>
    <table>
      <tr>
        <th>Nombre</th>
        <th>Ritmo Cardiaco (bpm)</th>
        <th>Temperatura Corporal (°C)</th>
        <th>Acciones</th>
      </tr>
      %PACIENTES%
    </table>
    <h2>Gráfica de Ritmo Cardiaco</h2>
    <canvas id="graficaRitmoCardiaco"></canvas>
    <h2>Último Usuario</h2>
    <p>%ULTIMO_USUARIO%</p>
    <h2>Cambiar Tema</h2>
    <button class="button" onclick="toggleTheme()">Cambiar Tema</button>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
      var ctx = document.getElementById('graficaRitmoCardiaco').getContext('2d');
      var graficaRitmoCardiaco = new Chart(ctx, {
          type: 'line',
          data: {
              labels: [%ETIQUETAS%],
              datasets: [{
                  label: 'Ritmo Cardiaco (bpm)',
                  data: [%DATOS%],
                  borderColor: 'rgba(75, 192, 192, 1)',
                  borderWidth: 1,
                  fill: false
              }]
          },
          options: {
              scales: {
                  x: { beginAtZero: true },
                  y: { beginAtZero: true }
              }
          }
      });

      function toggleTheme() {
          document.body.classList.toggle('theme-dark');
          document.body.classList.toggle('theme-light');
      }

      function reproducirAlerta() {
        var audio = document.getElementById('alertaAudio');
        audio.play();
      }

      document.addEventListener('DOMContentLoaded', function() {
        var filas = document.querySelectorAll('table tr');
        filas.forEach(function(fila) {
          var ritmo = parseFloat(fila.cells[1].innerText);
          var temp = parseFloat(fila.cells[2].innerText);

          if (ritmo > 120 || ritmo < 50 || temp > 37.5 || temp < 36) {
            fila.style.backgroundColor = '#ffdddd';
            reproducirAlerta();
          }
        });
      });
    </script>
    <audio id="alertaAudio" src="data:audio/wav;base64,UklGRi4AAABXQVZFZm10IBAAAAABAAEAgAIAEAAABAgAABAIwAgAABkBIAJ3AAEABAAEAAkCAAIABwAAADP/9qAAsrS7w+ZlBBSSkAAYoMY4JQAAQA2/gAAAAA=="></audio>
  </div>
</body>
</html>
)rawliteral";

// Función para manejar el callback del sensor
void onBeatDetected() {
    Serial.println("¡Latido detectado!");
}

// Función para manejar el envío del formulario
void manejarEnvio() {
    if (cantidadPacientes < 10) {
        String nombre = server.arg("nombre");
        float ritmoCardiaco = server.arg("ritmoCardiaco").toFloat();
        float tempCorporal = server.arg("tempCorporal").toFloat();

        if (nombre != "" && ritmoCardiaco > 0 && tempCorporal > 0) {
            pacientes[cantidadPacientes].nombre = nombre;
            pacientes[cantidadPacientes].ritmoCardiaco = ritmoCardiaco;
            pacientes[cantidadPacientes].tempCorporal = tempCorporal;
            cantidadPacientes++;
        }
    }
    // Registrar el usuario en el servidor intermediario (puede ser Google Sheets o similar)
    HTTPClient http;
    http.begin("https://script.google.com/macros/s/TU_ID_DE_SCRIPT/exec");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String payload = "usuario=" + ultimoUsuario + "&ritmoCardiaco=" + String(pacientes[cantidadPacientes - 1].ritmoCardiaco) + "&tempCorporal=" + String(pacientes[cantidadPacientes - 1].tempCorporal);
    int httpCode = http.POST(payload);
    if (httpCode > 0) {
        String respuesta = http.getString();
        Serial.println("Respuesta del servidor: " + respuesta);
    } else {
        Serial.println("Error en la solicitud HTTP");
    }
    http.end();
    server.sendHeader("Location", "/");
    server.send(303);
}

// Función para manejar la eliminación de un paciente
void manejarEliminar() {
    int indice = server.arg("indice").toInt();
    if (indice >= 0 && indice < cantidadPacientes) {
        for (int i = indice; i < cantidadPacientes - 1; i++) {
            pacientes[i] = pacientes[i + 1];
        }
        cantidadPacientes--;
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

// Función para autenticar al usuario
bool autenticar() {
    return server.authenticate(usuarioAdmin, claveAdmin);
}

// Función para mostrar la página principal
void mostrarPaginaPrincipal() {
    String pacientesHTML;
    String etiquetas;
    String datos;
    for (int i = 0; i < cantidadPacientes; i++) {
        pacientesHTML += "<tr><td>" + pacientes[i].nombre + "</td><td>" + String(pacientes[i].ritmoCardiaco) + "</td><td>" + String(pacientes[i].tempCorporal) + "</td><td><a href='/eliminar?indice=" + String(i) + "'>Eliminar</a></td></tr>";
        etiquetas += "'" + String(i) + "',";
        datos += String(pacientes[i].ritmoCardiaco) + ",";
    }
    if (etiquetas.length() > 0) etiquetas.remove(etiquetas.length() - 1);
    if (datos.length() > 0) datos.remove(datos.length() - 1);

    String pagina = paginaHTML;
    pagina.replace("%PACIENTES%", pacientesHTML);
    pagina.replace("%ULTIMO_USUARIO%", ultimoUsuario);
    pagina.replace("%ETIQUETAS%", etiquetas);
    pagina.replace("%DATOS%", datos);
    server.send(200, "text/html", pagina);
}

// Función para configurar las rutas del servidor
void configurarRutas() {
    server.on("/", HTTP_GET, []() {
        if (!autenticar()) {
            server.requestAuthentication();
            return;
        }
        mostrarPaginaPrincipal();
    });

    server.on("/agregar", HTTP_GET, []() {
        if (!autenticar()) {
            server.requestAuthentication();
            return;
        }
        server.send(200, "text/html", "<h1>Agregar Paciente</h1><form action='/enviar' method='post' class='formulario'><input type='text' name='nombre' placeholder='Nombre' required><input type='number' name='ritmoCardiaco' placeholder='Ritmo Cardiaco' required><input type='number' name='tempCorporal' placeholder='Temperatura Corporal' required><input type='submit' value='Agregar' class='button'></form>");
    });

    server.on("/ver", HTTP_GET, mostrarPaginaPrincipal);
    server.on("/eliminar", HTTP_GET, manejarEliminar);

    server.onNotFound([]() {
        server.send(404, "text/plain", "Página no encontrada");
    });
}

// Configuración inicial
void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Conectado a WiFi");

    pox.begin();
    ticker.attach_ms(REPORTING_PERIOD_MS, []() {
        pox.update();
        if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
            tsLastReport = millis();
        }
    });

    configurarRutas();

    server.begin();
    Serial.println("Servidor iniciado");
}

// Bucle principal
void loop() {
    server.handleClient();
    pox.update();
    if (pox.getHeartRate() > maxRitmoCardiaco || pox.getHeartRate() < minRitmoCardiaco || pox.getSpO2() > maxTempCorporal || pox.getSpO2() < minTempCorporal) {
        Serial.println("¡Alerta! Valores fuera de rango.");
    }
}

