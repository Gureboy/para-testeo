#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Biblioteca WiFiManager

ESP8266WebServer server(80);

String pacientes = "";

// Declaraciones de las funciones
void handleRoot();
void handleAgregar();
void handleVer();
void handleEliminar();
void handleEnviar();

void setup() {
  Serial.begin(115200);

  // Configurar WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("Saluduino-Config");

  Serial.println("Conectado a la red WiFi");

  server.on("/", handleRoot);
  server.on("/agregar", handleAgregar);
  server.on("/ver", handleVer);
  server.on("/eliminar", handleEliminar);
  server.on("/enviar", handleEnviar);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Saluduino - Sistema de Monitoreo</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #f3f4f6;
      margin: 0;
      padding: 0;
      color: #333;
    }

    .header {
      background-color: #242582;
      color: white;
      padding: 30px 20px;
      text-align: center;
      box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.1);
    }

    .header h1 {
      margin: 0;
      font-size: 2.5em;
      font-weight: 600;
    }

    .header p {
      font-size: 1.2em;
      margin-top: 10px;
    }

    .container {
      max-width: 1200px;
      margin: 20px auto;
      padding: 20px;
      background-color: #ffffff;
      border-radius: 10px;
      box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
    }

    .buttons {
      display: flex;
      justify-content: space-between;
      margin-bottom: 20px;
    }

    .button {
      flex: 1;
      padding: 15px;
      margin: 5px;
      border: none;
      border-radius: 8px;
      color: white;
      background-color: #4CAF50;
      cursor: pointer;
      font-size: 1em;
      transition: background-color 0.3s ease;
      text-align: center;
    }

    .button:hover {
      background-color: #3e8e41;
    }

    .formulario {
      margin-top: 20px;
      padding: 20px;
      background-color: #f9f9f9;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
    }

    .formulario label {
      display: block;
      margin-bottom: 10px;
      font-weight: bold;
      color: #242582;
    }

    .formulario input[type="text"],
    .formulario input[type="number"] {
      width: 100%;
      padding: 12px;
      margin-bottom: 20px;
      border: 1px solid #ddd;
      border-radius: 8px;
      box-sizing: border-box;
    }

    .formulario input[type="submit"] {
      background-color: #242582;
      color: white;
      padding: 15px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-size: 1em;
      transition: background-color 0.3s ease;
    }

    .formulario input[type="submit"]:hover {
      background-color: #1e2056;
    }

    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 30px;
    }

    table th, table td {
      padding: 15px;
      text-align: left;
      border-bottom: 1px solid #ddd;
    }

    table th {
      background-color: #f4f4f9;
      color: #242582;
    }

    table tr:nth-child(even) {
      background-color: #f9f9f9;
    }

    .alerta {
      color: red;
      font-weight: bold;
    }

    canvas {
      max-width: 100%;
      height: auto;
      margin-top: 30px;
    }

    @media (max-width: 768px) {
      .header h1 {
        font-size: 2em;
      }

      .header p {
        font-size: 1em;
      }

      .buttons {
        flex-direction: column;
        align-items: stretch;
      }

      .button {
        margin-bottom: 15px;
      }
    }
  </style>
</head>
<body>
  <div class="header">
    <h1>Saluduino - Monitoreo de Pacientes</h1>
    <p>Gestiona y monitorea la información de tus pacientes en tiempo real</p>
  </div>

  <div class="container">
    <div class="buttons">
      <button class="button" onclick="location.href='/agregar'">Agregar Paciente</button>
      <button class="button" onclick="location.href='/ver'">Ver Lista de Pacientes</button>
      <button class="button" onclick="location.href='/eliminar'">Eliminar Paciente</button>
    </div>

    <h2>Formulario de Datos del Paciente</h2>
    <form action="/enviar" method="post" class="formulario">
      <label for="nombre">Nombre del Paciente:</label>
      <input type="text" id="nombre" name="nombre" required>
      
      <label for="ritmoCardiaco">Ritmo Cardiaco (bpm):</label>
      <input type="number" id="ritmoCardiaco" name="ritmoCardiaco" step="0.1" required>
      
      <label for="tempCorporal">Temperatura Corporal (°C):</label>
      <input type="number" id="tempCorporal" name="tempCorporal" step="0.1" required>
      
      <input type="submit" value="Enviar">
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
            borderWidth: 2,
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
    </script>
  </div>
</body>
</html>
)rawliteral";

  html.replace("%PACIENTES%", pacientes);
  html.replace("%ETIQUETAS%", "['Enero', 'Febrero', 'Marzo']"); // Ejemplo
  html.replace("%DATOS%", "[65, 59, 80]"); // Ejemplo
  server.send(200, "text/html", html);
}

void handleAgregar() {
  server.send(200, "text/plain", "Agregar Paciente");
}

void handleVer() {
  server.send(200, "text/plain", "Ver Lista de Pacientes");
}

void handleEliminar() {
  server.send(200, "text/plain", "Eliminar Paciente");
}

void handleEnviar() {
  if (server.method() == HTTP_POST) {
    String nombre = server.arg("nombre");
    String ritmoCardiaco = server.arg("ritmoCardiaco");
    String tempCorporal = server.arg("tempCorporal");

    String nuevoPaciente = "<tr><td>" + nombre + "</td><td>" + ritmoCardiaco + "</td><td>" + tempCorporal + "</td><td><button onclick=\"location.href='/eliminar?nombre=" + nombre + "'\">Eliminar</button></td></tr>";
    pacientes += nuevoPaciente;

    server.sendHeader("Location", "/", true);
    server.send(303);
  } else {
    server.send(405, "text/plain", "Método no permitido");
  }
}
