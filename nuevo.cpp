#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Constantes y variables
const int TRIGGER_PIN = 0;   // Pin para iniciar la configuración
const int ledwifi = 12;      // LED indicador de conexión WiFi
const int leddisparo = 13;   // LED indicador de disparo
const int disparo = 4;       // Pin de entrada para el disparo

char telefono1[15] = "";     // Número de teléfono 1 para enviar mensajes
char apikey1[10] = "";       // API Key 1 para la API de CallMeBot
char telefono2[15] = "";     // Número de teléfono 2 para enviar mensajes
char apikey2[10] = "";       // API Key 2 para la API de CallMeBot

bool shouldSaveConfig = false;  // Bandera para indicar si se debe guardar la configuración

// Prototipos de funciones
void saveConfigCallback();
void sendMessage(const String& phoneNumber, const String& apiKey, const String& message);
void sendMessage1(const String& message);
void sendMessage2(const String& message);
void loadConfig();
void saveConfig();
void checkWiFiConnection();
void checkAndResetConfigPortal();

// Callback para guardar configuración cuando se modifica desde el portal WiFiManager
void saveConfigCallback() {
  shouldSaveConfig = true;
}

// Función setup: configuración inicial
void setup() {
  Serial.begin(115200);
  Serial.println();

  // Configuración de pines
  pinMode(TRIGGER_PIN, INPUT);    // Pin de entrada para activar el reinicio de configuración
  pinMode(ledwifi, OUTPUT);       // LED para indicar conexión WiFi
  pinMode(leddisparo, OUTPUT);    // LED para indicar disparo de alarma
  pinMode(disparo, INPUT);        // Pin de entrada para detectar el disparo de alarma

  // Cargar configuración desde el sistema de archivos SPIFFS
  loadConfig();

  // Inicializar el WiFiManager
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);  // Establecer callback para guardar configuración modificada
  wifiManager.setConfigPortalTimeout(180);  // Tiempo de espera para el portal de configuración

  // Parámetros personalizados para el portal de configuración
  WiFiManagerParameter custom_telefono1("telefono1", "Telefono 1", telefono1, 15);
  WiFiManagerParameter custom_apikey1("apikey1", "API Key 1", apikey1, 10);
  WiFiManagerParameter custom_telefono2("telefono2", "Telefono 2", telefono2, 15);
  WiFiManagerParameter custom_apikey2("apikey2", "API Key 2", apikey2, 10);

  // Agregar parámetros personalizados al WiFiManager
  wifiManager.addParameter(&custom_telefono1);
  wifiManager.addParameter(&custom_apikey1);
  wifiManager.addParameter(&custom_telefono2);
  wifiManager.addParameter(&custom_apikey2);

  // Intentar conectarse automáticamente al WiFi guardado
  checkWiFiConnection();

  // Guardar los valores de los parámetros personalizados después de la conexión
  strcpy(telefono1, custom_telefono1.getValue());
  strcpy(apikey1, custom_apikey1.getValue());
  strcpy(telefono2, custom_telefono2.getValue());
  strcpy(apikey2, custom_apikey2.getValue());

  // Guardar configuración si es necesario después de la modificación
  if (shouldSaveConfig) {
    saveConfig();
  }

  // Indicar que estamos conectados al WiFi
  Serial.println("Conectado al WiFi:");
  Serial.println(WiFi.localIP());
  digitalWrite(ledwifi, HIGH);  // Encender LED de conexión WiFi
}

// Función loop: bucle principal
void loop() {
  // Verificar y reiniciar el portal de configuración si es necesario
  checkAndResetConfigPortal();

  // Si se activa el pin de disparo, enviar mensajes de alarma
  if (digitalRead(disparo) == LOW) {
    digitalWrite(leddisparo, HIGH);
    sendMessage1("Disparo de Alarma !!!!!!");
    delay(5000);
    sendMessage2("Disparo de Alarma !!!!!!");
    delay(180000);  // Esperar 3 minutos antes de enviar otro mensaje
  } else {
    digitalWrite(leddisparo, LOW);  // Apagar LED de disparo si no hay alarma
  }
}

// Función para enviar un mensaje usando la API de CallMeBot
void sendMessage(const String& phoneNumber, const String& apiKey, const String& message) {
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("");

  if (httpResponseCode == 200) {
    Serial.println("Mensaje enviado exitosamente");
  } else {
    Serial.println("Error al enviar el mensaje");
    Serial.print("Código de respuesta HTTP: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// Función para enviar un mensaje al primer número de teléfono
void sendMessage1(const String& message) {
  sendMessage(String(telefono1), String(apikey1), message);
}

// Función para enviar un mensaje al segundo número de teléfono
void sendMessage2(const String& message) {
  sendMessage(String(telefono2), String(apikey2), message);
}

// Función para cargar la configuración desde SPIFFS
void loadConfig() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        if (!deserializeError) {
          strcpy(telefono1, json["telefono1"]);
          strcpy(apikey1, json["apikey1"]);
          strcpy(telefono2, json["telefono2"]);
          strcpy(apikey2, json["apikey2"]);
        }
        configFile.close();
      }
    }
  }
}

// Función para guardar la configuración en SPIFFS
void saveConfig() {
  DynamicJsonDocument json(1024);
  json["telefono1"] = telefono1;
  json["apikey1"] = apikey1;
  json["telefono2"] = telefono2;
  json["apikey2"] = apikey2;

  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    serializeJson(json, configFile);
    configFile.close();
  }
}

// Función para verificar la conexión WiFi y reconectar si es necesario
void checkWiFiConnection() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("Chap config portal", "");  // Intentar conexión automática

  // Guardar los valores de los parámetros personalizados después de la conexión
  strcpy(telefono1, WiFiManagerParameter("telefono1").getValue());
  strcpy(apikey1, WiFiManagerParameter("apikey1").getValue());
  strcpy(telefono2, WiFiManagerParameter("telefono2").getValue());
  strcpy(apikey2, WiFiManagerParameter("apikey2").getValue());

  // Indicar que estamos conectados al WiFi
  Serial.println("Conectado al WiFi:");
  Serial.println(WiFi.localIP());
  digitalWrite(ledwifi, HIGH);  // Encender LED de conexión WiFi
}

// Función para verificar y reiniciar el portal de configuración si es necesario
void checkAndResetConfigPortal() {
  if (digitalRead(TRIGGER_PIN) == LOW) {
    WiFiManager wm;
    wm.resetSettings();  // Reiniciar configuración del WiFiManager
    wm.setConfigPortalTimeout(120);  // Establecer tiempo de espera para el portal de configuración

    if (!wm.startConfigPortal("Whatsapp_Gateway")) {
      Serial.println("Fallo la conexión y se alcanzó el tiempo de espera");
      delay(3000);
      ESP.restart();  // Reiniciar ESP8266 en caso de fallo persistente
      delay(5000);
    }

    Serial.println("Conectado... OK :)");  // Confirmar conexión exitosa
    WiFi.begin(ssid, password);  // Intentar conexión WiFi

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("Conectado a la red WiFi con la dirección IP:");  // Mostrar IP asignada
    Serial.println(WiFi.localIP());
    digitalWrite(ledwifi, HIGH);  // Encender LED de conexión WiFi
  }
}