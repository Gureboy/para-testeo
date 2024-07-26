#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "tu_SSID";  // Reemplaza con tu SSID
const char* password = "tu_CONTRASEÑA";  // Reemplaza con tu contraseña

ESP8266WebServer server(80);

// Estructura para almacenar información del paciente
struct Paciente {
    String nombre;
    float ritmoCardiaco;
    float tempCorporal;
};

// Lista para almacenar pacientes (puedes ajustar el tamaño según sea necesario)
Paciente pacientes[10];
int numeroPacientes = 0;

// Página HTML con el formulario
const char* formularioHTML = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Datos del Paciente</title>
</head>
<body>
  <h2>Ingresar Datos del Paciente</h2>
  <form action="/submit" method="post">
    <label for="nombre">Nombre:</label><br>
    <input type="text" id="nombre" name="nombre"><br>
    <label for="ritmoCardiaco">Ritmo Cardiaco (bpm):</label><br>
    <input type="number" id="ritmoCardiaco" name="ritmoCardiaco" step="0.1"><br>
    <label for="tempCorporal">Temperatura Corporal (°C):</label><br>
    <input type="number" id="tempCorporal" name="tempCorporal" step="0.1"><br><br>
    <input type="submit" value="Enviar">
  </form>
  <h2>Pacientes Almacenados</h2>
  %PACIENTES%
</body>
</html>
)rawliteral";

// Función para procesar el formulario y almacenar los datos del paciente
void manejarSubmit() {
    if (numeroPacientes < 10) {
        pacientes[numeroPacientes].nombre = server.arg("nombre");
        pacientes[numeroPacientes].ritmoCardiaco = server.arg("ritmoCardiaco").toFloat();
        pacientes[numeroPacientes].tempCorporal = server.arg("tempCorporal").toFloat();
        numeroPacientes++;
    }
    server.sendHeader("Location", "/");
    server.send(303);
}

// Función para generar la lista de pacientes en HTML
String generarListaPacientes() {
    String listaPacientes = "";
    for (int i = 0; i < numeroPacientes; i++) {
        listaPacientes += "<p>";
        listaPacientes += "Nombre: " + pacientes[i].nombre + "<br>";
        listaPacientes += "Ritmo Cardiaco: " + String(pacientes[i].ritmoCardiaco) + " bpm<br>";
        listaPacientes += "Temperatura Corporal: " + String(pacientes[i].tempCorporal) + " °C<br>";
        listaPacientes += "</p>";
    }
    return listaPacientes;
}

// Función para manejar la página principal
void manejarRoot() {
    String pagina = formularioHTML;
    pagina.replace("%PACIENTES%", generarListaPacientes());
    server.send(200, "text/html", pagina);
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

    server.on("/", manejarRoot);
    server.on("/submit", HTTP_POST, manejarSubmit);
    server.begin();
    Serial.println("Servidor iniciado");
}

void loop() {
    server.handleClient();
}
