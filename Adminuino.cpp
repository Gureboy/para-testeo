#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Configura tus credenciales WiFi
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// Dirección del servidor Flask
const char* serverName = "http://TU_IP_DEL_SERVIDOR:5000/guardar_datos";

// ID del dispositivo
const char* deviceID = "esp8266_001";

// Variables para almacenar los datos a enviar
int sensorValue = 0;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
}

void loop() {
  // Simula la lectura de un sensor
  sensorValue = analogRead(A0);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Prepara el JSON
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["id"] = deviceID;
    jsonDoc["sensor"] = sensorValue;
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Realiza la solicitud HTTP POST
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.print("Error en la solicitud POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("Error en la conexión WiFi");
  }

  // Espera antes de enviar la siguiente lectura
  delay(60000); // Envía datos cada minuto
}
