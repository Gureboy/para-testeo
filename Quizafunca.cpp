#include <FS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Constantes y variables
const int TRIGGER_PIN = 0;   // Pin para iniciar la configuración
const int ledwifi = 12;      // LED indicador de conexión WiFi
const int leddisparo = 13;   // LED indicador de disparo
const int disparo = 4;       // Pin de entrada para el disparo
const int output4 = D4;      // Define el pin para GPIO 4
const int output5 = D5;      // Define el pin para GPIO 5

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
String urlEncode(const String &str);

// WiFiManager instance
WiFiManager wifiManager;

// Web server instance
ESP8266WebServer server(80);

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
  pinMode(output4, OUTPUT);       // Configura el pin de salida para GPIO 4
  pinMode(output5, OUTPUT);       // Configura el pin de salida para GPIO 5

  // Cargar configuración desde el sistema de archivos SPIFFS
  loadConfig();

  // Inicializar el WiFiManager
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
  if (!wifiManager.autoConnect("ChapConfigPortal")) {
    Serial.println("Fallo la conexión y se alcanzó el tiempo de espera");
    delay(3000);
    ESP.restart();  // Reiniciar ESP8266 en caso de fallo persistente
  }

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

  // Configurar servidor web para control de GPIOs
  server.on("/", handleRoot);
  server.on("/5/on", HTTP_GET, []() {
    digitalWrite(output5, HIGH);
    server.send(200, "text/plain", "GPIO 5 encendido");
  });
  server.on("/5/off", HTTP_GET, []() {
    digitalWrite(output5, LOW);
    server.send(200, "text/plain", "GPIO 5 apagado");
  });
  server.on("/4/on", HTTP_GET, []() {
    digitalWrite(output4, HIGH);
    server.send(200, "text/plain", "GPIO 4 encendido");
  });
  server.on("/4/off", HTTP_GET, []() {
    digitalWrite(output4, LOW);
    server.send(200, "text/plain", "GPIO 4 apagado");
  });

  server.begin();
}

// Función loop: bucle principal
void loop() {
  // Verificar y reiniciar el portal de configuración si es necesario
  wifiManager.process();
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

  // Manejo del servidor web
  server.handleClient();
}

// Función para enviar un mensaje usando la API de CallMeBot
void sendMessage(const String& phoneNumber, const String& apiKey, const String& message) {
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  Serial.println("URL: " + url); // Imprime la URL para depuración

  WiFiClient client;
  HTTPClient http;
  
  // Verificar conexión WiFi antes de enviar el mensaje
  if(WiFi.status() == WL_CONNECTED) {
    http.begin(client, url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST("");

    if (httpResponseCode > 0) {
      if (httpResponseCode == 200) {
        Serial.println("Mensaje enviado exitosamente");
      } else {
        Serial.println("Error al enviar el mensaje");
        Serial.print("Código de respuesta HTTP: ");
        Serial.println(httpResponseCode);
      }
    } else {
      Serial.print("Error en la solicitud HTTP: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Conexión WiFi no disponible");
  }
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
    }

    Serial.println("Conectado... OK :)");  // Indicar que estamos conectados al WiFi
  }
}

// Función para codificar una URL
String urlEncode(const String &str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

// Función para manejar la raíz del servidor web
void handleRoot() {
  // Implementa la lógica para manejar la raíz del servidor aquí
  server.send(200, "text/plain", "¡Bienvenido al servidor ESP8266!");
}
