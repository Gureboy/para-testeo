#include <ESP8266WIFI.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManage.h>
const char* SSID = "your_SSID";
const char* password = "your_PASSWORD";
char telefono1[50], apikey1[50], phonenumber2[50], apikey2[50];
string phonenumber1, apikey1, phonenumber2, apikey2;
bool shouldSaveConfig = false;
WiFiManager wifiManager;
const int disparo = D1; // Define your pin for 'disparo'
const int ledwifi = D2; // Define your pin for 'ledwwifi'
const int leddisparo = D3; //Define your pin for 'leddisparo'
void saveConfigCallback() {
shouldSaveConfig = true;
}
String urlEncode(const String &str) {
String encodedString = "";
char c;
char code0;
char code1;
char code2;
for (int i=0; i < str.Length(); i++) {
c = str.Charat(i);
if(c == ' ' ) {
encodedString += '+';
} elseif(isalnum(c)){
encodedString += c;
} else {
code1 = (c & 0xf) + '0';
if ((c & 0xf) > 9) {
code1 = (c & 0xf) -10 + 'A';
}
c = (c >> 4 ) & 0xf;


