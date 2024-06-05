#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
char telefono1[50], apikey1[50], telefono2[50], apikey2[50];
String phoneNumber1, apiKey1, phoneNumber2, apiKey2;
bool shouldSaveConfig = false;

const int disparo = D1; // Pin para 'disparo' button/input
const int ledwifi = D2; // Pin para WiFi status LED
const int leddisparo = D3; // Pin para 'disparo' status LED

WiFiManager wifiManager;

// Callback notificando la necesidad de guardar info
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Funcion para codificar URL a string y llamadas de char a string usando logica alfa numerica de c++
String urlEncode(const String &str) {
  String encodedString = "";
  char c;
  char code0;
  char code1;
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
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
    }
  }
  return encodedString;
}

// Funcion para mandar mensaje por el CallMeBot
void sendMessage(const String& phoneNumber, const String& apiKey, const String& message) {
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpResponseCode = http.POST("");
  if (httpResponseCode == 200) {
    Serial.println("Message sent successfully");
  } else {
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Funcion para cargar configuracion desde SPIFFS
void loadConfig() {
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        if (!deserializeError) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        if (json.success()) {
#endif
          // Copiar datos internos de un JSON a un array de chars
          strcpy(telefono1, json["telefono1"]);
          strcpy(apikey1, json["apikey1"]);
          strcpy(telefono2, json["telefono2"]);
          strcpy(apikey2, json["apikey2"]);
          // Conversion de array de char a String
          phoneNumber1 = String(telefono1);
          apiKey1 = String(apikey1);
          phoneNumber2 = String(telefono2);
          apiKey2 = String(apikey2);
        }
      }
    }
  }
}

// Funcion para guardar configuracion a SPIFFS
void saveConfig() {
  if (shouldSaveConfig) {
    Serial.println("Saving config");
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
    json["telefono1"] = phoneNumber1;
    json["apikey1"] = apiKey1;
    json["telefono2"] = phoneNumber2;
    json["apikey2"] = apiKey2;
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["telefono1"] = phoneNumber1;
    json["apikey1"] = apiKey1;
    json["telefono2"] = phoneNumber2;
    json["apikey2"] = apiKey2;
#endif
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("Failed to open config file for writing");
    } else {
#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
      serializeJson(json, configFile);
#else
      json.printTo(configFile);
#endif
      configFile.close();
    }
  }
}

void setup() {
  Serial.begin(9600); // Iniciar comunicacion de serial
  pinMode(disparo, INPUT_PULLUP); // Inicia el pin para 'disparo' boton/input
  pinMode(ledwifi, OUTPUT); // Inicia el pin para WiFi status LED
  pinMode(leddisparo, OUTPUT); // Inicia el pin para 'disparo' status LED
  digitalWrite(ledwifi, LOW); // apaga el led Wifi 
  digitalWrite(leddisparo, LOW); // apaga el led de 'disparo' 

  // Configurar el WiFiManager 
  wifiManager.setTimeout(120); // Tiempo de apagado para WiFiManager
  wifiManager.setSaveConfigCallback(saveConfigCallback); // Dar llamadas callback para guardar configuracion

  loadConfig(); // aca se carga la configuracion de los SPIFFS

  // Aca se conecta al WiFi usando el WiFiManager
  if (!wifiManager.autoConnect("Chap config portal", "")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart(); // Reinicio el ESP si la conexion falla 
    delay(5000);
  }

  Serial.println("Connected...yeey :)");
  digitalWrite(ledwifi, HIGH); // Apagar led wifi una vez que se conecta 

  Serial.println("Local IP:");
  Serial.println(WiFi.localIP()); // copiar la IP local, ojo lo podes usar como IPGrabber tambien esta condicion

  saveConfig(); // Guarda la configuracion de SPIFFS si es necesario 
}

void loop() {
  if (digitalRead(disparo) == LOW) { // Checkea si la funcion de 'disparo' boton/input esta activo
    digitalWrite(leddisparo, HIGH); // enciende el 'disparo' LED
    sendMessage(phoneNumber1, apiKey1, "Disparo de Alarma !!!!!!"); // manda el primer mensaje 
    delay(5000); // un delay de 5 segundos
    sendMessage(phoneNumber2, apiKey2, "Disparo de Alarma !!!!!!"); // segundo mensaje
    delay(180000); // el parametro de 'disparo' asi no rompe los quinotos
  } else {
    digitalWrite(leddisparo, LOW); // apaga el led de 'disparo' si el boton/input no se activa
  }
}

//Ahi ta todo maso menos explicado todo lo que hace cada cosa, probalo si funciona ;-B

