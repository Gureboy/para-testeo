#define BLYNK_TEMPLATE_ID "TMPL2OJd6sy3i"  // ID de la plantilla en Blynk
#define BLYNK_TEMPLATE_NAME "sistema de alarma lacs"  
#define BLYNK_AUTH_TOKEN "q-b5VTL5TTTuCI8-d6DgwWguNMVNyGCc"  // Token de autenticación para Blynk

#define BLYNK_PRINT Serial  


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

Ticker ticker;
WiFiManager wm;  // Para gestionar la conexión WiFi
BlynkTimer timer;  // Timer de Blynk para ejecutar acciones periódicas

// Pines a usar
#define ALARM_PIN D7  // Pin donde se conecta la alarma (sirena)
#define PIN_ACTIVAR D1  // Pin para activar/desactivar la alarma manualmente
#define SENSOR_PIN D2  // Pin conectado al sensor de movimiento
#define LED_WIFI D6  // LED para indicar el estado del WiFi

// Variables para manejar el estado de la alarma
bool alarmaActivada = false;
unsigned long ultimaAlerta = 0;
const unsigned long INTERVALO_ALERTA = 60000;  // Un minuto entre alertas
bool alarmaSonando = false;
const unsigned long TIEMPO_SONANDO = 10000;  // La alarma suena durante 10 segundos
unsigned long tiempoInicioAlarma = 0;
const unsigned long TIEMPO_REPIQUE = 500;  // Duración del "repique" de la sirena

// Variables para evitar notificaciones repetitivas
unsigned long ultimaNotificacion = 0;
const unsigned long INTERVALO_NOTIFICACION = 5000;  // Intervalo de 5 segundos entre notificaciones

// Función para hacer parpadear el LED de WiFi
void tick() {
  digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));  // Cambia el estado del LED
}

// Callback cuando el ESP está en modo de configuración WiFi (WiFiManager)
void configModeCallback(WiFiManager *myWiFiManager) {
  Serial.println("⚠️ Modo Configuración WiFi");  // Informa que está en modo configuración
  Serial.println(WiFi.softAPIP());  // Imprime la IP del ESP en modo AP
  Serial.println(myWiFiManager->getConfigPortalSSID());  // Imprime el SSID del portal de configuración
  ticker.attach(0.2, tick);  // Inicia el parpadeo del LED cada 200 ms
}

// Esta función maneja el botón virtual V2 en Blynk para activar o desactivar la alarma
BLYNK_WRITE(V2) {
  int alarmVal = param.asInt();  // Leemos el valor del botón (0 o 1)
  if (alarmVal == 0) {
    desactivar();  // Si se presiona el botón para desactivar
  } else {
    activar();  // Si se presiona el botón para activar
  }
  Blynk.virtualWrite(V2, alarmaActivada);  // Enviamos el estado de la alarma a Blynk
}

// Verifica el sensor de movimiento y controla la alarma
void verificarSensores() {
  int sensorEstado = digitalRead(SENSOR_PIN);  // Leemos el estado del sensor de movimiento
  // Si la alarma está activada, y detectamos movimiento, y no está sonando la alarma
  if (alarmaActivada && sensorEstado == HIGH && !alarmaSonando && millis() - ultimaAlerta > INTERVALO_ALERTA) {
    // Verifica si ha pasado el tiempo suficiente para enviar una nueva alerta
    if (millis() - ultimaNotificacion > INTERVALO_NOTIFICACION) {
      Serial.println("⚠️ ALERTA: Movimiento detectado!");
      ultimaNotificacion = millis();  // Actualiza la hora de la última notificación
      activarAlarma();  // Activa la alarma
      ultimaAlerta = millis();  // Actualiza la hora de la última alerta
    }
  }
  // Si la alarma está sonando y se ha cumplido el tiempo de duración
  if (alarmaSonando && millis() - tiempoInicioAlarma > TIEMPO_SONANDO) {
    desactivarAlarma();  // Desactiva la alarma
  }
}

// Activa la alarma (sirena)
void activarAlarma() {
  if (!alarmaSonando) {  // Si la alarma no está sonando
    alarmaSonando = true;  // La alarma pasa a estado sonando
    digitalWrite(ALARM_PIN, HIGH);  // Enciende la alarma (sirena)
    Serial.println("🔴 Alarma ACTIVADA");
    tiempoInicioAlarma = millis();  // Guarda el tiempo de inicio de la alarma
  }
}

// Desactiva la alarma
void desactivarAlarma() {
  alarmaSonando = false;  // La alarma deja de sonar
  digitalWrite(ALARM_PIN, LOW);  // Apaga la alarma (sirena)
  Serial.println("🚨 Alarma DESACTIVADA");
}

// Intenta reconectar con el servidor de Blynk si se pierde la conexión
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

// Verifica si el ESP8266 sigue conectado al WiFi, si no, intenta reconectar
void verificarWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Intentando reconectar...");
    WiFi.reconnect();  // Intenta reconectar el WiFi
  }
}

// Esta función se llama cuando se presiona el botón físico para cambiar el estado de la alarma
void ICACHE_RAM_ATTR cambiarEstadoAlarma() {
  alarmaActivada = !alarmaActivada;  // Cambia el estado de la alarma
  Serial.println(alarmaActivada ? "🔴 Alarma ACTIVADA" : "🚨 Alarma DESACTIVADA");
  repiqueSirena(alarmaActivada);  // Activa el repique de la sirena
  Blynk.virtualWrite(V2, alarmaActivada);  // Actualiza el estado en Blynk
}

// Repique de la sirena, dependiendo si se activa o desactiva
void repiqueSirena(bool activacion) {
  if (activacion) {
    digitalWrite(ALARM_PIN, HIGH);
    delay(TIEMPO_REPIQUE);
    digitalWrite(ALARM_PIN, LOW);
  } else {
    digitalWrite(ALARM_PIN, HIGH);
    delay(TIEMPO_REPIQUE);
    digitalWrite(ALARM_PIN, LOW);
    delay(TIEMPO_REPIQUE);
    digitalWrite(ALARM_PIN, HIGH);
    delay(TIEMPO_REPIQUE);
    digitalWrite(ALARM_PIN, LOW);
  }
}

// Función de configuración inicial
void setup() {
  Serial.begin(115200);  
  pinMode(ALARM_PIN, OUTPUT);  // Configura el pin de la alarma como salida
  pinMode(PIN_ACTIVAR, INPUT_PULLUP);  // Configura el pin para activar/desactivar la alarma como entrada
  pinMode(SENSOR_PIN, INPUT_PULLUP);  // Configura el pin del sensor de movimiento como entrada
  pinMode(LED_WIFI, OUTPUT);  // Configura el pin del LED de WiFi como salida
  attachInterrupt(digitalPinToInterrupt(PIN_ACTIVAR), cambiarEstadoAlarma, FALLING);  // Configura la interrupción para el botón
  digitalWrite(ALARM_PIN, LOW);  // Apaga la alarma al principio
  digitalWrite(LED_WIFI, HIGH);  // Enciende el LED de WiFi para indicar que está conectado
  wm.setConfigPortalTimeout(180);  // Tiempo límite para configurar WiFi
  wm.setAPCallback(configModeCallback);  // Configura el callback de configuración de WiFi
  if (!wm.autoConnect("AlarmaPerimetral", "clave123")) {  // Intenta conectar al WiFi usando WiFiManager
    Serial.println("⚠️ No se pudo conectar al WiFi, reiniciando...");
    ESP.restart();  // Si no se pudo conectar, reinicia el ESP
  }
  Serial.println("✅ Conectado a WiFi!");
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());  // Inicia Blynk con el token de autenticación
  timer.setInterval(1000L, verificarSensores);  // Verifica el sensor cada segundo
  timer.setInterval(30000L, verificarWiFi);  // Verifica la conexión WiFi cada 30 segundos
}

// Ciclo principal del programa
void loop() {
  if (Blynk.connected()) {  // Si está conectado a Blynk
    Blynk.run();  // Ejecuta las acciones de Blynk
  } else {
    reconectarBlynk();  // Si no está conectado, intenta reconectar
  }
  timer.run();  // Ejecuta el temporizador de Blynk
}

// Función para activar la alarma desde Blynk
void activar() {
  alarmaActivada = true;
  digitalWrite(ALARM_PIN, HIGH);  // Enciende la alarma
  Serial.println("🔴 Alarma ACTIVADA");
}
