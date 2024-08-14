#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <Arduino.h>

// Definición de variables para autenticación
const char* usuario = "admin";
const char* password = "admin";

// Crear un servidor web en el puerto 80
ESP8266WebServer server(80);

// Simular datos de pacientes
struct Paciente {
  int id;
  String nombre;
  int edad;
  float bpm;      // Datos del sensor MAX30100
  float temperatura; // Datos del sensor DS18B20
};

Paciente pacientes[] = {
  {1, "Paciente 1", 30, 75.0, 36.6},
  {2, "Paciente 2", 40, 80.0, 37.1}
};

// Función para manejar la página principal
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Inicio</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link" href="#"><i class="fas fa-bars"></i></a>
          </li>
          <li class="nav-item d-none d-sm-inline-block">
            <a href="/" class="nav-link">Inicio</a>
          </li>
        </ul>
      </nav>
    </header>
    <aside class="main-sidebar elevation-4">
      <a href="/" class="brand-link">
        <span class="brand-text font-weight-light">Saluduino</span>
      </a>
      <div class="sidebar">
        <nav class="mt-2">
          <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
            <li class="nav-item">
              <a href="/avanzadas" class="nav-link">
                <i class="nav-icon fas fa-cogs"></i>
                <p>Opciones Avanzadas</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/hub" class="nav-link">
                <i class="nav-icon fas fa-clinic-medical"></i>
                <p>Hub Enfermero</p>
              </a>
            </li>
          </ul>
        </nav>
      </div>
    </aside>
    <div class="content-wrapper">
      <div class="content-header">
        <div class="container-fluid">
          <h1 class="m-0 text-dark">Inicio</h1>
        </div>
      </div>
      <div class="content">
        <div class="container-fluid">
          <div class="row">
            <div class="col-md-4">
              <a href="/ver" class="btn btn-primary btn-lg btn-block">Ver Pacientes</a>
            </div>
            <div class="col-md-4">
              <a href="/agregar" class="btn btn-success btn-lg btn-block">Agregar Paciente</a>
            </div>
            <div class="col-md-4">
              <a href="/eliminar" class="btn btn-danger btn-lg btn-block">Eliminar Paciente</a>
            </div>
          </div>
        </div>
      </div>
    </div>
    <footer class="main-footer">
      <strong>Saluduino - Sistema de Monitoreo &copy; 2024</strong>
    </footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para manejar la página de agregar pacientes
void handleAgregar() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Agregar Paciente</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link" href="#"><i class="fas fa-bars"></i></a>
          </li>
          <li class="nav-item d-none d-sm-inline-block">
            <a href="/" class="nav-link">Inicio</a>
          </li>
        </ul>
      </nav>
    </header>
    <aside class="main-sidebar elevation-4">
      <a href="/" class="brand-link">
        <span class="brand-text font-weight-light">Saluduino</span>
      </a>
      <div class="sidebar">
        <nav class="mt-2">
          <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
            <li class="nav-item">
              <a href="/agregar" class="nav-link active">
                <i class="nav-icon fas fa-plus"></i>
                <p>Agregar Paciente</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/ver" class="nav-link">
                <i class="nav-icon fas fa-eye"></i>
                <p>Ver Pacientes</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/eliminar" class="nav-link">
                <i class="nav-icon fas fa-trash"></i>
                <p>Eliminar Paciente</p>
              </a>
            </li>
          </ul>
        </nav>
      </div>
    </aside>
    <div class="content-wrapper">
      <div class="content-header">
        <div class="container-fluid">
          <h1 class="m-0 text-dark">Agregar Paciente</h1>
        </div>
      </div>
      <div class="content">
        <div class="container-fluid">
          <form action="/agregar_paciente" method="post">
            <div class="form-group">
              <label for="nombre">Nombre:</label>
              <input type="text" id="nombre" name="nombre" class="form-control" required>
            </div>
            <div class="form-group">
              <label for="edad">Edad:</label>
              <input type="number" id="edad" name="edad" class="form-control" required>
            </div>
            <button type="submit" class="btn btn-success">Agregar</button>
          </form>
        </div>
      </div>
    </div>
    <footer class="main-footer">
      <strong>Saluduino - Sistema de Monitoreo &copy; 2024</strong>
    </footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para manejar la página de ver pacientes
void handleVer() {
  String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Ver Pacientes</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
    .table td, .table th {
      vertical-align: middle;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="

nav-link" href="#"><i class="fas fa-bars"></i></a>
</li>
<li class="nav-item d-none d-sm-inline-block">
<a href="/" class="nav-link">Inicio</a>
</li>
</ul>
</nav>
</header>
<aside class="main-sidebar elevation-4">
<a href="/" class="brand-link">
<span class="brand-text font-weight-light">Saluduino</span>
</a>
<div class="sidebar">
<nav class="mt-2">
<ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
<li class="nav-item">
<a href="/ver" class="nav-link active">
<i class="nav-icon fas fa-eye"></i>
<p>Ver Pacientes</p>
</a>
</li>
<li class="nav-item">
<a href="/agregar" class="nav-link">
<i class="nav-icon fas fa-plus"></i>
<p>Agregar Paciente</p>
</a>
</li>
<li class="nav-item">
<a href="/eliminar" class="nav-link">
<i class="nav-icon fas fa-trash"></i>
<p>Eliminar Paciente</p>
</a>
</li>
</ul>
</nav>
</div>
</aside>
<div class="content-wrapper">
<div class="content-header">
<div class="container-fluid">
<h1 class="m-0 text-dark">Lista de Pacientes</h1>
</div>
</div>
<div class="content">
<div class="container-fluid">
<table class="table table-bordered table-striped">
<thead>
<tr>
<th>ID</th>
<th>Nombre</th>
<th>Edad</th>
<th>BPM</th>
<th>Temperatura</th>
</tr>
</thead>
<tbody>
)rawliteral";

// Agregar los datos de pacientes
for (int i = 0; i < sizeof(pacientes) / sizeof(pacientes[0]); i++) {
html += "<tr>";
html += "<td>" + String(pacientes[i].id) + "</td>";
html += "<td>" + pacientes[i].nombre + "</td>";
html += "<td>" + String(pacientes[i].edad) + "</td>";
html += "<td>" + String(pacientes[i].bpm) + "</td>";
html += "<td>" + String(pacientes[i].temperatura) + "</td>";
html += "</tr>";
}

html += R"rawliteral(
</tbody>
</table>
</div>
</div>
</div>
<footer class="main-footer">
<strong>Saluduino - Sistema de Monitoreo © 2024</strong>
</footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para manejar la página de eliminar pacientes
void handleEliminar() {
String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Eliminar Paciente</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link" href="#"><i class="fas fa-bars"></i></a>
          </li>
          <li class="nav-item d-none d-sm-inline-block">
            <a href="/" class="nav-link">Inicio</a>
          </li>
        </ul>
      </nav>
    </header>
    <aside class="main-sidebar elevation-4">
      <a href="/" class="brand-link">
        <span class="brand-text font-weight-light">Saluduino</span>
      </a>
      <div class="sidebar">
        <nav class="mt-2">
          <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
            <li class="nav-item">
              <a href="/eliminar" class="nav-link active">
                <i class="nav-icon fas fa-trash"></i>
                <p>Eliminar Paciente</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/ver" class="nav-link">
                <i class="nav-icon fas fa-eye"></i>
                <p>Ver Pacientes</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/agregar" class="nav-link">
                <i class="nav-icon fas fa-plus"></i>
                <p>Agregar Paciente</p>
              </a>
            </li>
          </ul>
        </nav>
      </div>
    </aside>
    <div class="content-wrapper">
      <div class="content-header">
        <div class="container-fluid">
          <h1 class="m-0 text-dark">Eliminar Paciente</h1>
        </div>
      </div>
      <div class="content">
        <div class="container-fluid">
          <form action="/eliminar_paciente" method="post">
            <div class="form-group">
              <label for="id">ID del Paciente:</label>
              <input type="number" id="id" name="id" class="form-control" required>
            </div>
            <button type="submit" class="btn btn-danger">Eliminar</button>
          </form>
        </div>
      </div>
    </div>
    <footer class="main-footer">
      <strong>Saluduino - Sistema de Monitoreo &copy; 2024</strong>
    </footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para manejar la página de opciones avanzadas
void handleOpcionesAvanzadas() {
String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Opciones Avanzadas</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
    .btn-danger {
      background-color: #dc3545;
      border-color: #dc3545;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link" href="#"><i class="fas fa-bars"></i></a>
          </li>
          <li class="nav-item d-none d-sm-inline-block">
            <a href="/" class="nav-link">Inicio</a>
          </li>
        </ul>
      </nav>
    </header>
    <aside class="main-sidebar elevation-4">
      <a href="/" class="brand-link">
        <span class="brand-text font-weight-light">Saluduino</span>
      </a>
      <div class="sidebar">
        <nav class="mt-2">
          <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
            <li class="nav-item">
              <a href="/avanzadas" class="nav-link active">
                <i class="nav-icon fas fa-cogs"></i>
                <p>Opciones Avanzadas</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/hub" class="nav-link">
                <i class="nav-icon fas fa-clinic-medical"></i>
                <p>Hub Enfermero</p>
              </a>
            </li>
          </ul>
        </nav>
      </div>
    </aside>
    <div class="content-wrapper">
      <div class="content-header">
        <div class="container-fluid">
          <h1 class="m-0 text-dark">Opciones Avanzadas</h1>
        </div>
      </div>
      <div class="content">
        <div class="container-fluid">
          <h3 class="mb-4">Gestionar Dispositivos</h3>
          <form action="/agregar_dispositivo" method="post">
            <div class="form-group">
              <label for="ip">IP del Dispositivo:</label>
              <input type="text" id="ip" name="ip" class="form-control" placeholder="192.168.1.100" required>
            </div>
            <button type="submit" class="btn btn-primary">Agregar Dispositivo</button>
          </form>
          <hr>
          <h3 class="mb-4">Eliminar Dispositivos</h3>
          <form action="/eliminar_dispositivo" method="post">
            <div class="form-group">
              <label for="ip">IP del Dispositivo a Eliminar:</label>
              <input type="text" id="ip" name="ip" class="form-control" placeholder="192.168.1.100" required>
            </div>
            <button type="submit" class="btn btn-danger">Eliminar Dispositivo</button>
          </form>
        </div>
      </div>
    </div>
    <footer class="main-footer">
      <strong>Saluduino - Sistema de Monitoreo &copy; 2024</strong>
    </footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

// Función para manejar la página de hub enfermero
void handleHub() {
String html = R"rawliteral(
<!DOCTYPE HTML>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Hub Enfermero</title>
  <!-- AdminLTE CSS -->
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/plugins/fontawesome-free/css/all.min.css">
  <link rel="stylesheet" href="https://adminlte.io/themes/v3/dist/css/adminlte.min.css">
  <style>
    .content-wrapper {
      background-color: #f4f6f9;
    }
    .container {
      margin: 20px;
    }
  </style>
</head>
<body class="hold-transition sidebar-mini">
  <div class="wrapper">
    <header class="main-header">
      <nav class="navbar navbar-expand navbar-dark">
        <ul class="navbar-nav">
          <li class="nav-item">
            <a class="nav-link" href="#"><i class="fas fa-bars"></i></a>
          </li>
          <li class="nav-item d-none d-sm-inline-block">
            <a href="/" class="nav-link">Inicio</a>
          </li>
        </ul>
      </nav>
    </header>
    <aside class="main-sidebar elevation-4">
      <a href="/" class="brand-link">
        <span class="brand-text font-weight-light">Saluduino</span>
      </a>
      <div class="sidebar">
        <nav class="mt-2">
          <ul class="nav nav-pills nav-sidebar flex-column" data-widget="treeview" role="menu" data-accordion="false">
            <li class="nav-item">
              <a href="/hub" class="nav-link active">
                <i class="nav-icon fas fa-clinic-medical"></i>
                <p>Hub Enfermero</p>
              </a>
            </li>
            <li class="nav-item">
              <a href="/avanzadas" class="nav-link">
                <i class="nav-icon fas fa-cogs"></i>
                <p>Opciones Avanzadas</p>
              </a>
            </li>
          </ul>
        </nav>
      </div>
    </aside>
    <div class="content-wrapper">
      <div class="content-header">
        <div class="container-fluid">
          <h1 class="m-0 text-dark">Hub Enfermero</h1>
        </div>
      </div>
      <div class="content">
        <div class="container-fluid">
          <h3 class="mb-4">Generar Reporte</h3>
          <form action="/generar_reporte" method="post">
            <div class="form-group">
              <label for="fecha_inicio">Fecha de Inicio:</label>
              <input type="date" id="fecha_inicio" name="fecha_inicio" class="form-control" required>
            </div>
            <div class="form-group">
              <label for="fecha_fin">Fecha de Fin:</label>
              <input type="date" id="fecha_fin" name="fecha_fin" class="form-control" required>
            </div>
            <button type="submit" class="btn btn-primary">Generar Reporte</button>
          </form>
          <hr>
          <h3 class="mb-4">Asociar Pacientes</h3>
          <form action="/asociar_paciente" method="post">
            <div class="form-group">
              <label for="paciente_id">ID del Paciente:</label>
              <input type="number" id="paciente_id" name="paciente_id" class="form-control" required>
            </div>
            <div class="form-group">
              <label for="informacion">Información del Check-in:</label>
              <textarea id="informacion" name="informacion" class="form-control" rows="4" required></textarea>
            </div>
            <button type="submit" class="btn btn-success">Asociar Paciente</button>
          </form>
        </div>
      </div>
    </div>
    <footer class="main-footer">
      <strong>Saluduino - Sistema de Monitoreo &copy; 2024</strong>
    </footer>
  </div>
</body>
</html>
)rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
Serial.begin(115200);
WiFiManager wifiManager;
wifiManager.autoConnect("Saluduino");

if (!SPIFFS.begin()) {
Serial.println("Error al montar el sistema de archivos.");
return;
}

// Manejar autenticación
server.on("/", HTTP_GET, handleRoot);
server.on("/ver", HTTP_GET, handleVer);
server.on("/agregar", HTTP_GET, handleAgregar);
server.on("/eliminar", HTTP_GET, handleEliminar);
server.on("/avanzadas", HTTP_GET, handleOpcionesAvanzadas);
server.on("/hub", HTTP_GET, handleHub);
server.on("/agregar_paciente", HTTP_POST, {
String nombre = server.arg("nombre");
int edad = server.arg("edad").toInt();
// Aquí debes agregar la lógica para agregar el paciente
server.send(200, "text/html", "Paciente agregado");
});
server.on("/eliminar_paciente", HTTP_POST, {
int id = server.arg("id").toInt();
// Aquí debes agregar la lógica para eliminar el paciente
server.send(200, "text/html", "Paciente eliminado");
});
server.on("/agregar_dispositivo", HTTP_POST, {
String ip = server.arg("ip");
// Aquí debes agregar la lógica para agregar un dispositivo
server.send(200, "text/html", "Dispositivo agregado");
});
server.on("/eliminar_dispositivo", HTTP_POST, {
String ip = server.arg("ip");
// Aquí debes agregar la lógica para eliminar un dispositivo
server.send(200, "text/html", "Dispositivo eliminado");
});
server.on("/generar_reporte", HTTP_POST, {
String fechaInicio = server.arg("fecha_inicio");
String fechaFin = server.arg("fecha_fin");
// Aquí debes agregar la lógica para generar el reporte
server.send(200, "text/html", "Reporte generado");
});
server.on("/asociar_paciente", HTTP_POST, {
int pacienteId = server.arg("paciente_id").toInt();
String informacion = server.arg("informacion");
// Aquí debes agregar la lógica para asociar un paciente
server.send(200, "text/html", "Paciente asociado");
});

server.begin();
}

void loop() {
server.handleClient();
}
