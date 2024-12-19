#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <Ticker.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL2einD6e19"
#define BLYNK_TEMPLATE_NAME "Sistema Perimetral"
#define BLYNK_AUTH_TOKEN "OxOdz4K6aYFhAWFscgto1dGvR67gqAO3"

// Pines y variables
#define ALARM_PIN D7
#define SENSOR_PIN D2
#define ACTIVATION_PIN D1
#define LED_PIN D6

bool alarmActivated = false;
bool alarmTriggered = false;
unsigned long lastEventMillis = 0;
unsigned long eventInterval = 60000; // Intervalo para notificaciones (1 minuto)

// Servidor web
ESP8266WebServer server(80);

// Ticker para parpadear LEDs
Ticker ticker;

// Funciones auxiliares
void tick() {
    int state = digitalRead(LED_PIN);
    digitalWrite(LED_PIN, !state);
}

void configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("Entrando en modo configuración");
    Serial.println(WiFi.softAPIP());
    ticker.attach(0.2, tick);
}

// Guardar eventos en SPIFFS
void saveEvent(const String &event) {
    File logFile = SPIFFS.open("/events.txt", "a");
    if (logFile) {
        logFile.println(event);
        logFile.close();
    } else {
        Serial.println("Error guardando el evento.");
    }
}

// Mostrar historial de eventos
void handleShowLogs() {
    if (!server.authenticate("admin", "admin")) {
        return server.requestAuthentication();
    }
    String logs;
    File logFile = SPIFFS.open("/events.txt", "r");
    if (logFile) {
        while (logFile.available()) {
            logs += logFile.readStringUntil('\n') + "<br>";
        }
        logFile.close();
    }
    server.send(200, "text/html", "<h1>Historial de eventos</h1><p>" + logs + "</p>");
}

// Control de alarma desde la interfaz web
void handleControlAlarm() {
    if (!server.authenticate("admin", "admin")) {
        return server.requestAuthentication();
    }
    if (server.hasArg("state")) {
        String state = server.arg("state");
        if (state == "on") {
            digitalWrite(ALARM_PIN, HIGH);
            alarmActivated = true;
            saveEvent("Alarma activada desde la web.");
        } else {
            digitalWrite(ALARM_PIN, LOW);
            alarmActivated = false;
            saveEvent("Alarma desactivada desde la web.");
        }
    }
    server.send(200, "text/html", "<h1>Alarma actualizada</h1>");
}

void setup() {
    Serial.begin(115200);
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(SENSOR_PIN, INPUT_PULLUP);
    pinMode(ACTIVATION_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    // Configuración inicial de LEDs
    ticker.attach(0.6, tick);

    // Iniciar SPIFFS
    if (!SPIFFS.begin()) {
        Serial.println("Error iniciando SPIFFS");
        return;
    }

    // Configuración WiFi
    WiFiManager wifiManager;
    wifiManager.setAPCallback(configModeCallback);
    if (!wifiManager.autoConnect("Alarma_Perimetral")) {
        Serial.println("Error al conectar WiFi, reiniciando...");
        ESP.restart();
    }
    ticker.detach();
    digitalWrite(LED_PIN, LOW);

    // Iniciar Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());

    // Configuración del servidor web
    server.on("/logs", handleShowLogs);
    server.on("/control", handleControlAlarm);
    server.begin();
    Serial.println("Servidor web iniciado");
}

void loop() {
    Blynk.run();
    server.handleClient();

    bool sensorState = digitalRead(SENSOR_PIN);
    bool activationState = digitalRead(ACTIVATION_PIN);

    if (sensorState && activationState) {
        if (!alarmTriggered) {
            alarmTriggered = true;
            digitalWrite(ALARM_PIN, HIGH);
            saveEvent("Alarma disparada por sensor.");
            Blynk.notify("Intruso detectado. Alarma activada.");
        }
    } else {
        alarmTriggered = false;
        digitalWrite(ALARM_PIN, LOW);
    }

    if (millis() - lastEventMillis >= eventInterval && alarmTriggered) {
        lastEventMillis = millis();
        Blynk.notify("Recordatorio: Alarma activa.");
    }
}
