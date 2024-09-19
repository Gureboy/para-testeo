
#define BLYNK_TEMPLATE_ID "PoneelID"
#define BLYNK_DEVICE_NAME "Nombre_dispositivo"
#define BLYNK_AUTH_TOKEN "APIKEY"

#include <BlynkSimpleEsp8266.h>
#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

// Pines
int LEDGSM = 3;      // LED para estado GSM
int SIRENA = A0;
int PGM = 5;

// Configuracion para múltiples zonas
const int numZonas = 2; // Numero de zonas (podes aumentar este numero para mas zonas)
int Zonas[] = {9, 10};  // Pines de las zonas (Z1, Z2, etc.)
int LEDs[] = {4, 6};    // Pines de los LEDs correspondientes a cada zona (LEDZ1, LEDZ2, etc.)

// Variables del programa
bool activada = false;
int estadosZonas[numZonas];
unsigned long periodosonando = 120000;
unsigned long tiempoahora = 0;
int estadosirena = LOW;

// Token de autenticacion de blynk
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
  Serial.begin(9600);
  SIM800L.begin(9600);
  Blynk.begin(auth, "SSID", "Password");

  // Inicia  RCSwitch
  mySwitch.enableReceive(0);  // Receptor en pin #2
  
  // Configuracion de pines
  pinMode(LEDGSM, OUTPUT);
  pinMode(SIRENA, OUTPUT);
  pinMode(PGM, OUTPUT);

  // Configuracion de pines para cada zona
  for (int i = 0; i < numZonas; i++) {
    pinMode(Zonas[i], INPUT);
    pinMode(LEDs[i], OUTPUT);
  }
}

void loop() {
  Blynk.run();  // Abrir blynk para mantener la conexión

  controlarSirena();

  leerZonas();

  leerControlRemoto();

  manejarAlarma();
}

// Funcion para controlar la sirena
void controlarSirena() {
  if (estadosirena == HIGH) {
    digitalWrite(SIRENA, HIGH);
    if (millis() > tiempoahora + periodosonando) {
      estadosirena = LOW;  // Apaga la sirena después de cierto tiempo
    }
  } else {
    digitalWrite(SIRENA, LOW);
    digitalWrite(PGM, LOW);
  }
}

// Funcion para leer el estado de las zonas
void leerZonas() {
  for (int i = 0; i < numZonas; i++) {
    estadosZonas[i] = digitalRead(Zonas[i]);
    if (estadosZonas[i]) {
      digitalWrite(LEDs[i], HIGH);
    } else {
      digitalWrite(LEDs[i], LOW);
    }
  }
}

// Funcion para leer el control remoto
void leerControlRemoto() {
  if (mySwitch.available()) {
    long int value = mySwitch.getReceivedValue();
    if (value == 0) {
      Serial.println("Error de código");
    } else {
      Serial.print("Código recibido: ");
      Serial.println(value);
      
      if (value == 2308456 || value == 11729960) {
        activarAlarma();
      } else if (value == 2308452 || value == 11729956) {
        desactivarAlarma();
      }
    }
    mySwitch.resetAvailable();
  }
}

// Función para manejar la activacion de la alarma
void manejarAlarma() {
  for (int i = 0; i < numZonas; i++) {
    if (estadosZonas[i] && activada) {
      estadosirena = HIGH;
      tiempoahora = millis();
      digitalWrite(PGM, HIGH);
    }
  }

  if (!activada || !hayZonasActivas()) {
    estadosirena = LOW;
    digitalWrite(PGM, LOW);
  }
}

// Función para activar la alarma
void activarAlarma() {
  activada = true;
  Serial.println("Alarma activada");
  digitalWrite(LEDGSM, HIGH);
  parpadearSirena();
  Blynk.notify("Alarma activada");  // Notificación en la app de Blynk
}

// Función para desactivar la alarma
void desactivarAlarma() {
  activada = false;
  estadosirena = LOW;
  Serial.println("Alarma desactivada");
  digitalWrite(LEDGSM, LOW);
  parpadearSirena();
  Blynk.notify("Alarma desactivada");  // Notificación en la app de Blynk
}

// Función para parpadear la sirena
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
    if (estadosZonas[i]) {
      return true;
    }
  }
  return false;
}
