#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "tu_SSID";  // Reemplaza con tu SSID
const char* password = "tu_CONTRASEÑA";  // Reemplaza con tu contraseña

ESP8266WebServer servidor(80);

// Estructura para almacenar información del paciente
struct Paciente {
    String nombre;
    float ritmoCardiaco;
    float tempCorporal;
};

// Lista para almacenar pacientes (puedes ajustar el tamaño según sea necesario)
Paciente pacientes[10];
int cantidadPacientes = 0;

// Página HTML con el formulario
const char* formularioHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Datos del Paciente</title>
  <style>
    body { font-family: Arial, sans-serif; }
    form { margin-bottom: 20px; }
    label { display: block; margin-top: 10px; }
    input[type="text"], input[type="number"] { width: 100%; padding: 8px; margin-top: 5px; }
    input[type="submit"] { padding: 10px 15px; margin-top: 10px; }
    table { width: 100%; border-collapse: collapse; margin-top: 20px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
    th { background-color: #f2f2f2; }
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

// Función para generar la lista de pacientes en HTML
String generarListaPacientes() {
    String listaPacientes = "";
    for (int i = 0; i < cantidadPacientes; i++) {
        listaPacientes += "<tr>";
        listaPacientes += "<td>" + pacientes[i].nombre + "</td>";
        listaPacientes += "<td>" + String(pacientes[i].ritmoCardiaco) + " bpm</td>";
        listaPacientes += "<td>" + String(pacientes[i].tempCorporal) + " °C</td>";
        listaPacientes += "<td><a href=\"/eliminar?indice=" + String(i) + "\">Eliminar</a></td>";
        listaPacientes += "</tr>";
    }
    return listaPacientes;
}

// Función para manejar la página principal
void manejarRaiz() {
    String pagina = formularioHTML;
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

