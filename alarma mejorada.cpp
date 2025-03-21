#define BLYNK_TEMPLATE_ID "TMPL2OJd6sy3i"
#define BLYNK_TEMPLATE_NAME "sistema de alarma lacs"
#define BLYNK_AUTH_TOKEN "q-b5VTL5TTTuCI8-d6DgwWguNMVNyGCc"

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

Ticker ticker;
WiFiManager wm;
BlynkTimer timer;

#define ALARM_PIN D7
#define PIN_ACTIVAR D1
#define SENSOR_PIN D2
#define LED_WIFI D6

bool alarmaActivada = false;
unsigned long ultimaAlerta = 0;
const unsigned long INTERVALO_ALERTA = 5000; // Reducido a 5 segundos
bool alarmaSonando = false;
const unsigned long TIEMPO_SONANDO = 60000; // 1 minuto
unsigned long tiempoInicioAlarma = 0;

void tick() {
  digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));
}

void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("⚠️ Modo Configuración WiFi");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}

BLYNK_WRITE(V2) {
  int alarmVal = param.asInt();
  if (alarmVal == 0) {
    desactivar();
  } else {
    activar();
  }
}

void verificarSensores() {
  int sensorEstado = digitalRead(SENSOR_PIN);
  if (alarmaActivada && sensorEstado == HIGH && !alarmaSonando && millis() - ultimaAlerta > INTERVALO_ALERTA) {
    Serial.println("⚠️ ALERTA: Movimiento detectado!");
    activarAlarma();
    ultimaAlerta = millis();
  }
  if (alarmaSonando && millis() - tiempoInicioAlarma > TIEMPO_SONANDO) {
    desactivarAlarma();
  }
}

void activarAlarma() {
  if (!alarmaSonando) {
    alarmaSonando = true;
    digitalWrite(ALARM_PIN, HIGH);
    Serial.println("🔴 Alarma ACTIVADA");
    tiempoInicioAlarma = millis();
  }
}

void desactivarAlarma() {
  alarmaSonando = false;
  digitalWrite(ALARM_PIN, LOW);
  ultimaAlerta = millis(); // Reseteamos para permitir una nueva activación inmediata
  Serial.println("🚨 Alarma DESACTIVADA");
}

void reconectarBlynk() {
  if (!Blynk.connected()) {
    Serial.println("⚠️ Blynk desconectado. Intentando reconectar...");
    if (Blynk.connect()) {
      Serial.println("✅ Reconexion exitosa!");
    } else {
      Serial.println("❌ Fallo en la reconexión");
    }
  }
}

void verificarWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Intentando reconectar...");
    WiFi.reconnect();
  }
  if (millis() - ultimaAlerta > 300000 && WiFi.status() == WL_CONNECTED) {
    Serial.println("🛑 No hay actividad. Desactivando WiFi para ahorrar energía...");
    WiFi.mode(WIFI_OFF);
  }
}

void ICACHE_RAM_ATTR cambiarEstadoAlarma() {
  alarmaActivada = !alarmaActivada;
  Serial.println(alarmaActivada ? "🔴 Alarma ACTIVADA" : "🚨 Alarma DESACTIVADA");
}

void setup() {
  Serial.begin(115200);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(PIN_ACTIVAR, INPUT_PULLUP);
  pinMode(SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_WIFI, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ACTIVAR), cambiarEstadoAlarma, FALLING);
  digitalWrite(ALARM_PIN, LOW);
  digitalWrite(LED_WIFI, HIGH);
  wm.setConfigPortalTimeout(180);
  wm.setAPCallback(configModeCallback);
  if (!wm.autoConnect("AlarmaPerimetral", "clave123")) {
    Serial.println("⚠️ No se pudo conectar al WiFi, reiniciando...");
    ESP.restart();
  }
  Serial.println("✅ Conectado a WiFi!");
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
  timer.setInterval(1000L, verificarSensores);
  timer.setInterval(30000L, verificarWiFi);
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  } else {
    reconectarBlynk();
  }
  timer.run();
}

void activar() {
  alarmaActivada = true;
  Serial.println("🔴 Alarma ACTIVADA");
}

void desactivar() {
  alarmaActivada = false;
  desactivarAlarma();
  Serial.println("🚨 Alarma DESACTIVADA");
}
