#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <Ticker.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

const char* ssid = "tu_SSID";  // Reemplaza con tu SSID
const char* password = "tu_CONTRASEÑA";  // Reemplaza con tu contraseña

const char* usuario = "admin";
const char* clave = "1234";

ESP8266WebServer servidor(80);
const char* urlServidor = "https://script.google.com/macros/s/TU_ID_DE_SCRIPT/exec"; // Reemplaza con tu URL del script

const float maxRitmoCardiaco = 120.0;
const float minRitmoCardiaco = 50.0;
const float maxTempCorporal = 37.5;
const float minTempCorporal = 36.0;

struct Paciente {
    String nombre;
    float ritmoCardiaco;
    float tempCorporal;
};

Paciente pacientes[10];
int cantidadPacientes = 0;
String ultimoUsuario = "Ninguno";

PulseOximeter pox;
Ticker ticker;

#define REPORTING_PERIOD_MS 1000
uint32_t tsLastReport = 0;

const char* formularioHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Datos del Paciente</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 0; background-color: #f4f4f9; }
    header { background-color: #6200ea; color: white; padding: 15px 20px; text-align: center; }
    .container { padding: 20px; }
    form { background-color: white; padding: 20px; border-radius: 5px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    label { display: block; margin-top: 10px; }
    input[type="text"], input[type="number"] { width: calc(100% - 22px); padding: 10px; margin-top: 5px; border: 1px solid #ccc; border-radius: 4px; }
    input[type="submit"] { padding: 10px 15px; margin-top: 20px; background-color: #6200ea; color: white; border: none; border-radius: 4px; cursor: pointer; }
    input[type="submit"]:hover { background-color: #3700b3; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #6200ea; color: white; }
    .alerta { color: red; font-weight: bold; }
    canvas { max-width: 100%; height: auto; margin-top: 20px; }
    footer { background-color: #6200ea; color: white; text-align: center; padding: 10px 0; position: fixed; bottom: 0; width: 100%; }
  </style>
  <script>
    function validarFormulario() {
      var nombre = document.getElementById('nombre').value;
      var ritmoCardiaco = parseFloat(document.getElementById('ritmoCardiaco').value);
      var tempCorporal = parseFloat(document.getElementById('tempCorporal').value);

      if (!nombre) {
        alert("El nombre es obligatorio.");
        return false;
      }
      if (isNaN(ritmoCardiaco) || ritmoCardiaco <= 0) {
        alert("Por favor, ingrese un ritmo cardíaco válido.");
        return false;
      }
      if (isNaN(tempCorporal) || tempCorporal <= 0) {
        alert("Por favor, ingrese una temperatura corporal válida.");
        return false;
      }
      return true;
    }

    function filtrarPacientes() {
      var nombre = document.getElementById('filtroNombre').value.toLowerCase();
      var ritmo = parseFloat(document.getElementById('filtroRitmo').value);
      var temp = parseFloat(document.getElementById('filtroTemp').value);
      var filas = document.querySelectorAll('table tr');

      filas.forEach(function(fila, index) {
        if (index === 0) return;
        var nombreFila = fila.cells[0].innerText.toLowerCase();
        var ritmoFila = parseFloat(fila.cells[1].innerText);
        var tempFila = parseFloat(fila.cells[2].innerText);
        var mostrar = true;

        if (nombre && !nombreFila.includes(nombre)) mostrar = false;
        if (!isNaN(ritmo) && ritmoFila !== ritmo) mostrar = false;
        if (!isNaN(temp) && tempFila !== temp) mostrar = false;

        fila.style.display = mostrar ? '' : 'none';
      });
    }

    function editarPaciente(indice) {
      var nombre = prompt("Ingrese el nuevo nombre:");
      var ritmoCardiaco = prompt("Ingrese el nuevo ritmo cardiaco:");
      var tempCorporal = prompt("Ingrese la nueva temperatura corporal:");
      if (nombre && ritmoCardiaco && tempCorporal) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/editar", true);
        xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        xhr.send("indice=" + indice + "&nombre=" + nombre + "&ritmoCardiaco=" + ritmoCardiaco + "&tempCorporal=" + tempCorporal);
        xhr.onreadystatechange = function() {
          if (xhr.readyState == 4 && xhr.status == 200) {
            location.reload();
          }
        };
      }
    }

    function eliminarPaciente(indice) {
      if (confirm("¿Está seguro de que desea eliminar este paciente?")) {
        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/eliminar", true);
        xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        xhr.send("indice=" + indice);
        xhr.onreadystatechange = function() {
          if (xhr.readyState == 4 && xhr.status == 200) {
            location.reload();
          }
        };
      }
    }

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
</head>
<body>
  <header>
    <h1>Monitor de Pacientes</h1>
  </header>
  <div class="container">
    <h2>Ingresar Datos del Paciente</h2>
    <form action="/enviar" method="post" onsubmit="return validarFormulario()">
      <label for="nombre">Nombre:</label>
      <input type="text" id="nombre" name="nombre" required>
      <label for="ritmoCardiaco">Ritmo Cardiaco (bpm):</label>
      <input type="number" id="ritmoCardiaco" name="ritmoCardiaco" step="0.1" required>
      <label for="tempCorporal">Temperatura Corporal (°C):</label>
      <input type="number" id="tempCorporal" name="tempCorporal" step="0.1" required>
      <input type="submit" value="Enviar">
    </form>

    <h2>Filtrar Pacientes</h2>
    <label for="filtroNombre">Nombre:</label>
    <input type="text" id="filtroNombre" onkeyup="filtrarPacientes()">
    <label for="filtroRitmo">Ritmo Cardiaco (bpm):</label>
    <input type="number" id="filtroRitmo" step="0.1" onkeyup="filtrarPacientes()">
    <label for="filtroTemp">Temperatura Corporal (°C):</label>
    <input type="number" id="filtroTemp" step="0.1" onkeyup="filtrarPacientes()">

    <h2>Datos de Pacientes</h2>
    <table>
      <thead>
        <tr>
          <th>Nombre</th>
          <th>Ritmo Cardiaco (bpm)</th>
          <th>Temperatura Corporal (°C)</th>
          <th>Acciones</th>
        </tr>
      </thead>
      <tbody>
        %PACIENTES%
      </tbody>
    </table>

    <canvas id="graficaRitmoCardiaco"></canvas>
  </div>
  <audio id="alertaAudio" src="data:audio/mp3;base64,EAAEAAAB..."></audio>
  <footer>
    <p>&copy; 2024 Monitor de Pacientes</p>
  </footer>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi");

    if (!pox.begin()) {
        Serial.println("Fallo al iniciar el sensor PulseOximeter");
        while (true);
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    ticker.attach_ms(1000, []() {
        pox.update();
    });

    servidor.on("/", HTTP_GET, []() {
        if (!servidor.authenticate(usuario, clave)) {
            return servidor.requestAuthentication();
        }
        String contenido = formularioHTML;
        String pacientesHTML = "";
        String etiquetas = "";
        String datos = "";

        for (int i = 0; i < cantidadPacientes; i++) {
            Paciente p = pacientes[i];
            pacientesHTML += "<tr><td>" + p.nombre + "</td><td>" + String(p.ritmoCardiaco) + "</td><td>" + String(p.tempCorporal) + "</td><td><button onclick='editarPaciente(" + String(i) + ")'>Editar</button><button onclick='eliminarPaciente(" + String(i) + ")'>Eliminar</button></td></tr>";
            etiquetas += "'" + p.nombre + "',";
            datos += String(p.ritmoCardiaco) + ",";
        }
        contenido.replace("%PACIENTES%", pacientesHTML);
        contenido.replace("%ETIQUETAS%", etiquetas);
        contenido.replace("%DATOS%", datos);
        contenido.replace("%ULTIMO_USUARIO%", ultimoUsuario);

        servidor.send(200, "text/html", contenido);
    });

    servidor.on("/enviar", HTTP_POST, []() {
        if (!servidor.authenticate(usuario, clave)) {
            return servidor.requestAuthentication();
        }
        if (cantidadPacientes >= 10) {
            servidor.send(500, "text/html", "<html><body><h2>No se pueden agregar más pacientes</h2><p><a href='/'>Volver</a></p></body></html>");
            return;
        }
        String nombre = servidor.arg("nombre");
        float ritmoCardiaco = servidor.arg("ritmoCardiaco").toFloat();
        float tempCorporal = servidor.arg("tempCorporal").toFloat();
        ultimoUsuario = nombre;
        pacientes[cantidadPacientes++] = {nombre, ritmoCardiaco, tempCorporal};

        if (ritmoCardiaco > maxRitmoCardiaco || ritmoCardiaco < minRitmoCardiaco || tempCorporal > maxTempCorporal || tempCorporal < minTempCorporal) {
            // Lógica de alerta aquí
        }

        HTTPClient http;
        http.begin(urlServidor);
        http.addHeader("Content-Type", "application/json");
        String payload = "{\"nombre\":\"" + nombre + "\",\"ritmoCardiaco\":" + String(ritmoCardiaco) + ",\"tempCorporal\":" + String(tempCorporal) + "}";
        int httpCode = http.POST(payload);
        if (httpCode > 0) {
            String respuesta = http.getString();
            Serial.println("Respuesta del servidor: " + respuesta);
            servidor.send(200, "text/html", "<html><body><h2>Datos enviados exitosamente</h2><p><a href='/'>Volver</a></p></body></html>");
        } else {
            Serial.println("Error en la solicitud HTTP");
            servidor.send(500, "text/html", "<html><body><h2>Error en el envío de datos</h2><p><a href='/'>Volver</a></p></body></html>");
        }
        http.end();
    });

    servidor.on("/editar", HTTP_POST, []() {
        if (!servidor.authenticate(usuario, clave)) {
            return servidor.requestAuthentication();
        }
        int indice = servidor.arg("indice").toInt();
        String nombre = servidor.arg("nombre");
        float ritmoCardiaco = servidor.arg("ritmoCardiaco").toFloat();
        float tempCorporal = servidor.arg("tempCorporal").toFloat();

        if (indice >= 0 && indice < cantidadPacientes) {
            pacientes[indice] = {nombre, ritmoCardiaco, tempCorporal};
            servidor.send(200, "text/html", "<html><body><h2>Paciente editado exitosamente</h2><p><a href='/'>Volver</a></p></body></html>");
        } else {
            servidor.send(500, "text/html", "<html><body><h2>Índice de paciente no válido</h2><p><a href='/'>Volver</a></p></body></html>");
        }
    });

    servidor.on("/eliminar", HTTP_POST, []() {
        if (!servidor.authenticate(usuario, clave)) {
            return servidor.requestAuthentication();
        }
        int indice = servidor.arg("indice").toInt();
        if (indice >= 0 && indice < cantidadPacientes) {
            for (int i = indice; i < cantidadPacientes - 1; i++) {
                pacientes[i] = pacientes[i + 1];
            }
            cantidadPacientes--;
            servidor.send(200, "text/html", "<html><body><h2>Paciente eliminado exitosamente</h2><p><a href='/'>Volver</a></p></body></html>");
        } else {
            servidor.send(500, "text/html", "<html><body><h2>Índice de paciente no válido</h2><p><a href='/'>Volver</a></p></body></html>");
        }
    });

    servidor.begin();
    Serial.println("Servidor iniciado");
}

void loop() {
    servidor.handleClient();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        tsLastReport = millis();
        float ritmoCardiaco = pox.getHeartRate();
        float spO2 = pox.getSpO2();
        Serial.print("Ritmo Cardiaco: ");
        Serial.print(ritmoCardiaco);
        Serial.print(" bpm / SpO2: ");
        Serial.print(spO2);
        Serial.println(" %");
    }
}
