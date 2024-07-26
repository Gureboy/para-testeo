#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

const char* ssid = "tu_SSID";  // Reemplaza con tu SSID
const char* password = "tu_CONTRASEÑA";  // Reemplaza con tu contraseña

// Autenticación básica
const char* usuario = "admin";
const char* clave = "1234";

ESP8266WebServer servidor(80);

// Configuración de alertas
const float maxRitmoCardiaco = 120.0;
const float minRitmoCardiaco = 50.0;
const float maxTempCorporal = 37.5;
const float minTempCorporal = 36.0;

struct Paciente {
    String nombre;
    float ritmoCardiaco;
    float tempCorporal;
};

// Lista para almacenar pacientes (tamaño ajustable)
Paciente pacientes[10];
int cantidadPacientes = 0;

// Página HTML con el formulario
const char* formularioHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Datos del Paciente</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial, sans-serif; }
    form { margin-bottom: 20px; }
    label { display: block; margin-top: 10px; }
    input[type="text"], input[type="number"] { width: 100%; padding: 8px; margin-top: 5px; }
    input[type="submit"] { padding: 10px 15px; margin-top: 10px; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #f2f2f2; }
    .alerta { color: red; font-weight: bold; }
    canvas { max-width: 100%; height: auto; }
  </style>
</head>
<body>
  <h2>Ingresar Datos del Paciente</h2>
  <form action="/enviar" method="post">
    <label for="nombre">Nombre:</label>
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
  <audio id="alertaAudio" src="data:audio/wav;base64,UklGRi4AAABXQVZFZm10IBAAAAABAAEAgAIAEAAABAgAABAIwAgAABkBIAJ3AAEABAAEAAkCAAIABwAAADP/9qAAsrS7w+ZlBBSSkAAYoMY4JQAAQA2/gAAAAA=="></audio>
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
</body>
</html>
)rawliteral";

// Función para procesar el formulario y almacenar los datos del paciente
void manejarEnvio() {
    if (cantidadPacientes < 10) {
        String nombre = servidor.arg("nombre");
        float ritmoCardiaco = servidor.arg("ritmoCardiaco").toFloat();
        float tempCorporal = servidor.arg("tempCorporal").toFloat();

        if (nombre != "" && ritmoCardiaco > 0 && tempCorporal > 0) {
            pacientes[cantidadPacientes].nombre = nombre;
            pacientes[cantidadPacientes].ritmoCardiaco = ritmoCardiaco;
            pacientes[cantidadPacientes].tempCorporal = tempCorporal;
            cantidadPacientes++;
        }
    }
    servidor.sendHeader("Location", "/");
    servidor.send(303);
}

// Función para manejar la eliminación de un paciente
void manejarEliminar() {
    int indice = servidor.arg("indice").toInt();
    if (indice >= 0 && indice < cantidadPacientes) {
        for (int i = indice; i < cantidadPacientes - 1; i++) {
            pacientes[i] = pacientes[i + 1];
        }
        cantidadPacientes--;
    }
    servidor.sendHeader("Location", "/");
    servidor.send(303);
}

// Función para manejar la autenticación
bool autenticar() {
    if (!servidor.authenticate(usuario, clave)) {
        servidor.requestAuthentication();
        return false;
    }
    return true;
}

// Función para generar la lista de pacientes en HTML
String generarListaPacientes() {
    String listaPacientes = "";
    for (int i = 0; i < cantidadPacientes; i++) {
        listaPacientes += "<tr>";
        listaPacientes += "<td>" + pacientes[i].nombre + "</td>";
        listaPacientes += "<td class=\"" + (pacientes[i].ritmoCardiaco > maxRitmoCardiaco || pacientes[i].ritmoCardiaco < minRitmoCardiaco ? "alerta" : "") + "\">" + String(pacientes[i].ritmoCardiaco) + " bpm</td>";
        listaPacientes += "<td class=\"" + (pacientes[i].tempCorporal > maxTempCorporal || pacientes[i].tempCorporal < minTempCorporal ? "alerta" : "") + "\">" + String(pacientes[i].tempCorporal) + " °C</td>";
        listaPacientes += "<td><a href=\"/eliminar?indice=" + String(i) + "\">Eliminar</a></td>";
        listaPacientes += "</tr>";
    }
    return listaPacientes;
}

// Función para manejar la página principal
void manejarRaiz() {
    if (!autenticar()) return;

    String pagina = formularioHTML;
    // Datos para la gráfica
    String etiquetas = "";
    String datos = "";
    for (int i = 0; i < cantidadPacientes; i++) {
        if (i > 0) {
            etiquetas += ", ";
            datos += ", ";
        }
        etiquetas += "\"" + pacientes[i].nombre + "\"";
        datos += String(pacientes[i].ritmoCardiaco);
    }
    pagina.replace("%ETIQUETAS%", etiquetas);
    pagina.replace("%DATOS%", datos);
    pagina.replace("%PACIENTES%", generarListaPacientes());
    servidor.send(200, "text/html", pagina);
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado a WiFi");

    servidor.on("/", manejarRaiz);
    servidor.on("/enviar", HTTP_POST, manejarEnvio);
    servidor.on("/eliminar", manejarEliminar);
    servidor.begin();
    Serial.println("Servidor iniciado");
}

void loop() {
    servidor.handleClient();
}
