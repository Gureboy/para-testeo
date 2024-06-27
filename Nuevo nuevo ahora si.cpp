#include <FS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Constantes y variables
const int TRIGGER_PIN = 0;      
const int ledwifi = 12;         
const int leddisparo = 13;      
const int disparo = 4;          

char telefono1[15] = "";        // Variable para almacenar el número de teléfono 1
char apikey1[10] = "";          // Variable para almacenar la API Key 1
char telefono2[15] = "";        // Variable para almacenar el número de teléfono 2
char apikey2[10] = "";          // Variable para almacenar la API Key 2

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

// Callback para guardar configuración
void saveConfigCallback() {
  shouldSaveConfig = true;
}

// Función setup
void setup() {
  Serial.begin(115200); 
  Serial.println();

  // Configurar pines como entrada o salida
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(ledwifi, OUTPUT);
  pinMode(leddisparo, OUTPUT);
  pinMode(disparo, INPUT);

  // Cargar configuración desde el sistema de archivos
  loadConfig();

  // Iniciar WiFiManager
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);  // Configurar callback para guardar configuración
  wifiManager.setConfigPortalTimeout(180);                // Configurar tiempo de espera del portal de configuración

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

  // Verificar y establecer conexión WiFi
  checkWiFiConnection();

  // Copiar valores de parámetros personalizados a las variables correspondientes
  strcpy(telefono1, custom_telefono1.getValue());
  strcpy(apikey1, custom_apikey1.getValue());
  strcpy(telefono2, custom_telefono2.getValue());
  strcpy(apikey2, custom_apikey2.getValue());

  // Guardar configuración si es necesario
  if (shouldSaveConfig) {
    saveConfig();
  }

  // Información de conexión WiFi
  Serial.println("Conectado al WiFi:");
  Serial.println(WiFi.localIP());
  digitalWrite(ledwifi, HIGH);  // Encender LED indicador de conexión WiFi
}

// Función loop
void loop() {
  // Verificar y reiniciar portal de configuración si es necesario
  checkAndResetConfigPortal();

  // Verificar si se activa el disparo y enviar mensajes de alarma
  if (digitalRead(disparo) == LOW) {
    digitalWrite(leddisparo, HIGH);  // Encender LED indicador de disparo
    sendMessage1("Disparo de Alarma !!!!!!");
    delay(5000);
    sendMessage2("Disparo de Alarma !!!!!!");
    delay(180000);
  } else {
    digitalWrite(leddisparo, LOW);  // Apagar LED indicador de disparo
  }
}

// Función para enviar un mensaje usando la API de CallMeBot
void sendMessage(const String& phoneNumber, const String& apiKey, const String& message) {
  try {
    // Construir URL para enviar el mensaje por WhatsApp usando CallMeBot
    String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);  // Iniciar conexión HTTP
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // Agregar encabezado HTTP
    int httpResponseCode = http.POST("");  // Enviar solicitud POST vacía

    // Verificar el código de respuesta HTTP
    if (httpResponseCode == 200) {
      Serial.println("Mensaje enviado exitosamente");
    } else {
      Serial.println("Error al enviar el mensaje");
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Finaliza conexión HTTP
  } catch (const std::exception& e) {
    Serial.println("Excepción al enviar el mensaje:");
    Serial.println(e.what());  // Imprimir mensaje de error de excepción
    // registrar el error en un archivo de registro aquí
  }
}

// Función para enviar un mensaje al primer número de teléfono
void sendMessage1(const String& message) {
  sendMessage(String(telefono1), String(apikey1), message);  // Llamar a sendMessage con los valores del primer número de teléfono
}

// Función para enviar un mensaje al segundo número de teléfono
void sendMessage2(const String& message) {
  sendMessage(String(telefono2), String(apikey2), message);  // Llamar a sendMessage con los valores del segundo número de teléfono
}

// Función para cargar la configuración desde SPIFFS
void loadConfig() {
  try {
    // Iniciar sistema de archivos SPIFFS
    if (SPIFFS.begin()) {
      // Verificar si existe el archivo de configuración
      if (SPIFFS.exists("/config.json")) {
        // Abrir archivo de configuración en modo lectura
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          // Leer contenido del archivo y cargar en JSON
          size_t size = configFile.size();
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          DynamicJsonDocument json(1024);
          auto deserializeError = deserializeJson(json, buf.get());  // Deserializar JSON
          if (!deserializeError) {
            // Copiar valores del JSON a las variables correspondientes
            strcpy(telefono1, json["telefono1"]);
            strcpy(apikey1, json["apikey1"]);
            strcpy(telefono2, json["telefono2"]);
            strcpy(apikey2, json["apikey2"]);
          }
          configFile.close();  // Cerrar archivo de configuración
        }
      }
    }
  } catch (const std::exception& e) {
    Serial.println("Excepción al cargar la configuración:");
    Serial.println(e.what());  // Imprimir mensaje de error de excepción
    // Registra el error en un archivo de registro aquí
  }
}

// Función para guardar la configuración en SPIFFS
void saveConfig() {
  try {
    // Crear JSON con la configuración a guardar
    DynamicJsonDocument json(1024);
    json["telefono1"] = telefono1;
    json["apikey1"] = apikey1;
    json["telefono2"] = telefono2;
    json["apikey2"] = apikey2;

    // Abrir archivo de configuración en modo escritura
    File configFile = SPIFFS.open("/config.json", "w");
    if (configFile) {
      serializeJson(json, configFile);  // Serializar JSON y escribir en archivo
      configFile.close();  // Cerrar archivo de configuración
    }
  } catch (const std::exception& e) {
    Serial.println("Excepción al guardar la configuración:");
    Serial.println(e.what());  // Imprimir mensaje de error de excepción
    // Podrías registrar el error en un archivo de registro aquí
  }
}

// Función para verificar la conexión WiFi y reconectar si es necesario
void checkWiFiConnection() {
  try {
    WiFiManager wifiManager;  // Iniciar WiFiManager
    WiFi.mode(WIFI_STA);      // Configurar modo estación WiFi
    wifiManager.autoConnect("Whatsapp_Gateway");  // Conectar automáticamente al WiFi guardado

    // Imprimir mensaje de conexión exitosa y dirección IP local
    Serial.println("Conectado a la red WiFi con la dirección IP:");
    Serial.println(WiFi.localIP());
    digitalWrite(ledwifi, HIGH);  // Encender LED indicador de conexión WiFi
  } catch (const std::exception& e) {
    Serial.println("Excepción al conectar WiFi:");
    Serial.println(e.what());  // Imprimir mensaje de error de excepción
    // Podrías registrar el error en un archivo de registro aquí
  }
}

// Función para verificar y reiniciar el portal de configuración si se activa el pin de disparo
void checkAndResetConfigPortal() {
  if (digitalRead(TRIGGER_PIN) == LOW) {  // Verificar si se activa el pin de disparo
    WiFiManager wifiManager;  // Iniciar WiFiManager
    wifiManager.resetSettings();  // Restablecer configuración WiFi guardada
    wifiManager.setConfigPortalTimeout(120);  // Configurar tiempo de espera del portal de configuración

    // Iniciar portal de configuración y esperar conexión
    if (!wifiManager.startConfigPortal("Whatsapp_Gateway")) {
      Serial.println("Fallo la conexión y se alcanzó el tiempo de espera");
      delay(3000);
      ESP.restart();  // Reiniciar ESP8266 si falla la conexión
      delay(5000);
    }

    Serial.println("Conectado... OK :)");  // Mensaje de conexión
    WiFi.begin();  // Intentar conectar WiFi nuevamente
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");  // Imprimir puntos durante la conexión
    }
    Serial.println("Conectado a la red WiFi con la dirección IP:");
    Serial.println(WiFi.localIP());
    digitalWrite(ledwifi, HIGH);  // Encender LED indicador de conexión WiFi
  }
}
