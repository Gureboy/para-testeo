#define BLYNK_TEMPLATE_ID "PoneelID"
#define BLYNK_DEVICE_NAME "Nombre_dispositivo"
#define BLYNK_AUTH_TOKEN "APIKEY"

#include <BlynkSimpleEsp8266.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

// Pines
const int LEDGSM = 3;      // LED para estado GSM
const int SIRENA = A0;     // Pin de la sirena
const int PGM = 5;         // Pin de activación de PGM (programable)

// Configuración para múltiples zonas
const int numZonas = 2;    // Número de zonas (podes aumentar este número para más zonas)
int Zonas[] = {9, 10};     // Pines de las zonas (Z1, Z2, etc.)
int LEDs[] = {4, 6};       // Pines de los LEDs correspondientes a cada zona (LEDZ1, LEDZ2, etc.)

// Variables del programa
bool alarmaActivada = false;
int estadosZonas[numZonas];        // Estado actual de cada zona
unsigned long periodoSonido = 120000;  // Duración del sonido de la sirena
unsigned long tiempoInicioSonido = 0;  // Momento en que la sirena empezó a sonar
int estadoSirena = LOW;               // Estado actual de la sirena

// Token de autenticación de Blynk
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
  Serial.begin(9600);
  
  // Inicia SIM800L (si lo usas)
  SIM800L.begin(9600);
  
  // Inicia Blynk
  Blynk.begin(auth, "SSID", "Password");

  // Inicia RCSwitch
  mySwitch.enableReceive(0);  // Receptor en pin #2
  
  // Configuración de los pines
  pinMode(LEDGSM, OUTPUT);
  pinMode(SIRENA, OUTPUT);
  pinMode(PGM, OUTPUT);

  // Configuración de los pines de zonas y LEDs
  for (int i = 0; i < numZonas; i++) {
    pinMode(Zonas[i], INPUT);
    pinMode(LEDs[i], OUTPUT);
  }
}

void loop() {
  Blynk.run();  // Mantener la conexión a Blynk activa

  controlarSirena();
  leerZonas();
  leerControlRemoto();
  manejarAlarma();
}

// Función para controlar la sirena
void controlarSirena() {
  if (estadoSirena == HIGH) {
    digitalWrite(SIRENA, HIGH);
    if (millis() > tiempoInicioSonido + periodoSonido) {
      estadoSirena = LOW;  // Apagar la sirena después del periodo
    }
  } else {
    digitalWrite(SIRENA, LOW);
    digitalWrite(PGM, LOW);
  }
}

// Función para leer el estado de las zonas
void leerZonas() {
  for (int i = 0; i < numZonas; i++) {
    estadosZonas[i] = digitalRead(Zonas[i]);
    digitalWrite(LEDs[i], estadosZonas[i]);  // Encender o apagar el LED según el estado de la zona
  }
}

// Función para leer el control remoto
void leerControlRemoto() {
  if (mySwitch.available()) {
    long int valorRecibido = mySwitch.getReceivedValue();
    if (valorRecibido == 0) {
      Serial.println("Error de código");
    } else {
      Serial.print("Código recibido: ");
      Serial.println(valorRecibido);
      
      // Valores del control remoto para activar y desactivar la alarma
      if (valorRecibido == 2308456 || valorRecibido == 11729960) {
        activarAlarma();
      } else if (valorRecibido == 2308452 || valorRecibido == 11729956) {
        desactivarAlarma();
      }
    }
    mySwitch.resetAvailable();
  }
}

// Función para manejar la activación de la alarma
void manejarAlarma() {
  if (alarmaActivada && hayZonasActivas()) {
    estadoSirena = HIGH;
    tiempoInicioSonido = millis();
    digitalWrite(PGM, HIGH);
  } else if (!alarmaActivada || !hayZonasActivas()) {
    estadoSirena = LOW;
    digitalWrite(PGM, LOW);
  }
}

// Función para activar la alarma
void activarAlarma() {
  alarmaActivada = true;
  Serial.println("Alarma activada");
  digitalWrite(LEDGSM, HIGH);
  parpadearSirena();
  Blynk.notify("Alarma activada");  // Notificación a la app de Blynk
}

// Función para desactivar la alarma
void desactivarAlarma() {
  alarmaActivada = false;
  estadoSirena = LOW;
  Serial.println("Alarma desactivada");
  digitalWrite(LEDGSM, LOW);
  parpadearSirena();
  Blynk.notify("Alarma desactivada");  // Notificación a la app de Blynk
}

// Función para hacer parpadear la sirena al activar/desactivar la alarma
void parpadearSirena() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(SIRENA, HIGH);
    delay(200);
    digitalWrite(SIRENA, LOW);
    delay(200);
  }
}

// Función para verificar si alguna zona está activa
bool hayZonasActivas() {
  for (int i = 0; i < numZonas; i++) {
    if (estadosZonas[i] == HIGH) {
      return true;
    }
  }
  return false;
}
