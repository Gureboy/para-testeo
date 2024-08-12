#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Ticker.h>
#include <.h>

// Configuración de autenticación
const char* username = "admin";
const char* password = "password";

// Configuración del servidor web
ESP8266WebServer server(80);

// Variables para almacenar los datos
String pacientes = "";
String ipDevices = "";

// Función para autenticación
bool authenticate() {
  if (!server.authenticate(username, password)) {
    server.requestAuthentication();
    return false;
  }
  return true;
}

// Handler principal
void handleRoot() {
  if (!authenticate()) return;

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
    .boton-rojo {
      color: white;
      background-color: red;
      border: none;
      padding: 10px;
      border-radius: 8px;
      cursor: pointer;
    }
    .boton-rojo:hover {
      background-color: darkred;
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
      <button class="button" onclick="location.href='/avanzadas'">Opciones Avanzadas</button>
      <button class="button" onclick="location.href='/hub'">Hub Enfermero</button>
    </div>

    <h2>Formulario de Datos del Paciente</h2>
    <form action="/enviar" method="post" class="formulario">
      <label for="nombre">Nombre del Paciente:</label>
      <input type="text" id="nombre" name="nombre" required>
      
      <label for="apellido">Apellido del Paciente:</label>
      <input type="text" id="apellido" name="apellido" required>
      
      <label for="edad">Edad del Paciente:</label>
      <input type="number" id="edad" name="edad" required>
      
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
        <th>Apellido</th>
        <th>Edad</th>
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

// Handler para Agregar Paciente
void handleAgregar() {
  if (!authenticate()) return;

  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Agregar Paciente</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f4;
    }
    .container {
      max-width: 600px;
      margin: 30px auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      margin-top: 0;
      color: #333;
    }
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: bold;
    }
    input[type="text"],
    input[type="number"] {
      width: calc(100% - 22px);
      padding: 10px;
      margin-bottom: 12px;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    input[type="submit"] {
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      background-color: #28a745;
      color: white;
      font-size: 16px;
      cursor: pointer;
    }
    input[type="submit"]:hover {
      background-color: #218838;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Agregar Paciente</h2>
    <form action="/agregarPaciente" method="POST">
      <label for="nombre">Nombre del Paciente:</label>
      <input type="text" id="nombre" name="nombre" required>
      
      <label for="apellido">Apellido del Paciente:</label>
      <input type="text" id="apellido" name="apellido" required>
      
      <label for="edad">Edad del Paciente:</label>
      <input type="number" id="edad" name="edad" required>
      
      <label for="ritmoCardiaco">Ritmo Cardiaco (bpm):</label>
      <input type="number" id="ritmoCardiaco" name="ritmoCardiaco" step="0.1" required>
      
      <label for="tempCorporal">Temperatura Corporal (°C):</label>
      <input type="number" id="tempCorporal" name="tempCorporal" step="0.1" required>
      
      <input type="submit" value="Enviar">
    </form>
  </div>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

// Handler para Ver Pacientes
void handleVer() {
  if (!authenticate()) return;

  // Obtener pacientes y datos para mostrar
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Ver Pacientes</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f4;
    }
    .container {
      max-width: 800px;
      margin: 30px auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      margin-top: 0;
      color: #333;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 20px;
    }
    table th, table td {
      padding: 10px;
      text-align: left;
      border-bottom: 1px solid #ddd;
    }
    table th {
      background-color: #f4f4f9;
      color: #333;
    }
    table tr:nth-child(even) {
      background-color: #f9f9f9;
    }
    .boton-rojo {
      color: white;
      background-color: red;
      border: none;
      padding: 10px;
      border-radius: 8px;
      cursor: pointer;
    }
    .boton-rojo:hover {
      background-color: darkred;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Lista de Pacientes</h2>
    <table>
      <tr>
        <th>Nombre</th>
        <th>Apellido</th>
        <th>Edad</th>
        <th>Ritmo Cardiaco (bpm)</th>
        <th>Temperatura Corporal (°C)</th>
        <th>Acciones</th>
      </tr>
      %PACIENTES%
    </table>
  </div>
</body>
</html>
)rawliteral";

  html.replace("%PACIENTES%", pacientes);
  server.send(200, "text/html", html);
}

// Handler para Eliminar Paciente
void handleEliminar() {
  if (!authenticate()) return;

  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Eliminar Paciente</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f4;
    }
    .container {
      max-width: 600px;
      margin: 30px auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      margin-top: 0;
      color: #333;
    }
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: bold;
    }
    select {
      width: calc(100% - 22px);
      padding: 10px;
      margin-bottom: 12px;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    .boton-rojo {
      color: white;
      background-color: red;
      border: none;
      padding: 10px;
      border-radius: 4px;
      cursor: pointer;
    }
    .boton-rojo:hover {
      background-color: darkred;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Eliminar Paciente</h2>
    <form action="/eliminarPaciente" method="POST">
      <label for="paciente">Seleccione el Paciente a Eliminar:</label>
      <select id="paciente" name="paciente" required>
        %PACIENTES_SELECT%
      </select>
      <input type="submit" value="Eliminar" class="boton-rojo">
    </form>
  </div>
</body>
</html>
)rawliteral";

  // Aquí deberás generar el HTML de pacientes para el select
  String pacientesSelect = ""; // Aquí deberás agregar el código para generar opciones del select
  html.replace("%PACIENTES_SELECT%", pacientesSelect);
  server.send(200, "text/html", html);
}

// Handler para Opciones Avanzadas
void handleAvanzadas() {
  if (!authenticate()) return;

  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Opciones Avanzadas</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f4;
    }
    .container {
      max-width: 800px;
      margin: 30px auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      margin-top: 0;
      color: #333;
    }
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: bold;
    }
    input[type="text"] {
      width: calc(100% - 22px);
      padding: 10px;
      margin-bottom: 12px;
      border: 1px solid #ddd;
      border-radius: 4px;
    }
    input[type="submit"] {
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      background-color: #28a745;
      color: white;
      font-size: 16px;
      cursor: pointer;
    }
    input[type="submit"]:hover {
      background-color: #218838;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>Opciones Avanzadas</h2>
    <form action="/agregarIP" method="POST">
      <label for="ip">Agregar IP del Dispositivo ESP8266:</label>
      <input type="text" id="ip" name="ip" placeholder="Ejemplo: 192.168.1.100" required>
      <input type="submit" value="Agregar IP">
    </form>

    <h2>Dispositivos Asociados</h2>
    <ul>
      %IP_LIST%
    </ul>
  </div>
</body>
</html>
)rawliteral";

  // Aquí deberás generar la lista de IPs asociadas
  String ipList = ""; // Aquí deberás agregar el código para generar la lista de IPs
  html.replace("%IP_LIST%", ipList);
  server.send(200, "text/html", html);
}

// Handler para Hub Enfermero
void handleHub() {
  if (!authenticate()) return;

  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Hub Enfermero</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f4f4f4;
    }
    .container {
      max-width: 800px;
      margin: 30px auto;
      padding: 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    }
    h2 {
      margin-top: 0;
      color: #333;
    }
    .welcome-message {
      margin-bottom: 20px;
      font-size: 1.2em;
      color: #333;
    }
    .button {
      padding: 10px 15px;
      border: none;
      border-radius: 4px;
      background-color: #007bff;
      color: white;
      font-size: 16px;
      cursor: pointer;
      text-align: center;
    }
    .button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="welcome-message">Bienvenido, Enfermero %NOMBRE%</div>
    <form action="/descargarExcel" method="POST">
      <input type="submit" value="Descargar Registro del Día" class="button">
    </form>
  </div>
</body>
</html>
)rawliteral";

  String nombreEnfermero = "Nombre Enfermero"; // Aquí deberás obtener el nombre del enfermero
  html.replace("%NOMBRE%", nombreEnfermero);
  server.send(200, "text/html", html);
}

// Función para guardar datos de pacientes en archivo CSV
void saveToCSV(String filename, String content) {
  File file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println("Error abriendo el archivo CSV");
    return;
  }
  file.println(content);
  file.close();
}

// Configuración del servidor
void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;

  // Configurar el punto de acceso y guardar la configuración
  wifiManager.autoConnect("ESP8266_AP");

  if (SPIFFS.begin()) {
    Serial.println("Sistema de archivos SPIFFS montado correctamente");
  } else {
    Serial.println("Error al montar el sistema de archivos SPIFFS");
  }

  // Rutas de los handlers
  server.on("/", handleRoot);
  server.on("/agregar", handleAgregar);
  server.on("/ver", handleVer);
  server.on("/eliminar", handleEliminar);
  server.on("/avanzadas", handleAvanzadas);
  server.on("/hub", handleHub);

  server.begin();
  Serial.println("Servidor web iniciado");
}

void loop() {
  server.handleClient();
}
