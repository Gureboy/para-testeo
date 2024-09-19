RCSwitch mySwitch = RCSwitch();

//variables de pines
int LEDZ1 = 4;
int LEDGSM = 3;                  //ponemos en el pin 4 el LED gsm
int SIRENA = A0;
int Z1 = 9;
int PGM = 5;
// fin definicion de pines
//------------variables de programa-----------------
bool activada = false;
int estadoz1 = 0;
//char numerotel[] = "+5492664402440"; // numero de telefono
//char numerotel2[] = "+5492665034938"; // numero de telefono
//unsigned long tiempo;
unsigned long periodosonando = 120000;
unsigned long tiempoahora = 0;
int estadosirena;//


void setup() {
  //  Serial.begin(9600);
  SIM800L.begin(9600);
  mySwitch.enableReceive(0);  // Receptor en el pin #2
  pinMode(LEDGSM, OUTPUT);
  pinMode(LEDZ1, OUTPUT);
  pinMode(SIRENA, OUTPUT);
  pinMode(Z1, INPUT);
  pinMode (PGM, OUTPUT);
}
void loop() {

  if (estadosirena == HIGH) {
    digitalWrite (SIRENA , HIGH);

  }
  if (estadosirena == LOW ) {
    digitalWrite (SIRENA , LOW);
    digitalWrite(PGM, LOW);
  }





  //-----------------rutina de llamadas---------------

  /*    Serial.println("LLAMANDO A NUMERO UNO");

      digitalWrite(LEDGSM, HIGH);
      SIM800L.println("ATD+ +5492664310468;");      //Comando AT para llamar

      delay(10000);                                         //Delay de duracion de la primer llamada
      SIM800L.println("ATH");                                //Colgar
      Serial.println("FIN A LLAMADA NUMERO UNO");
      delay(10000);                                           //Delay de espera entre llamadas

      Serial.println("LLAMANDO A NUMERO DOS");
      SIM800L.println("ATD+ +5492664320026;");            //Comando AT para llamar

      delay(10000);                                       //Delay de duracion de la segunda llamada
      SIM800L.println("ATH");                            //Colgar
      Serial.println("FIN A LLAMADA NUMERO DOS");
      digitalWrite(LEDGSM, LOW);
      delay(1000);
  */
  //---------------------rutina de lamadas fin----------------------






  //------------Rutina para manejo de tiempo de sirena---------------

  if (estadosirena == LOW ) {
    digitalWrite (SIRENA , LOW);

  }
  if (estadosirena == HIGH  ) {
    if (millis() > tiempoahora + periodosonando) {
      estadosirena = LOW;


    }
  }





  //-------------------rutina de manejo de led indicador de zona-----------------


  estadoz1 = digitalRead (Z1);

  if (estadoz1) {
    digitalWrite (LEDZ1, HIGH);
  }
  else
  {
    digitalWrite (LEDZ1, LOW);
  }

  //-----------------------rutina de lectura de control remoto---------------------


  if (mySwitch.available()) {              // si tenemos datos disponibles que recibir

    long int value = mySwitch.getReceivedValue();

    if (value == 0)
    {
      Serial.print("error de codigo");   //Por si se recibe algo raro
    }
    else
    {
      Serial.print("codigo recibido ");
      Serial.println(value);
    }
   //  if (value == 9464680)
    if (value == 2308456)
    {
      Serial.println("Alarma activada");

      digitalWrite(LEDGSM, HIGH);
      activar();

    }

   // if (value == 9464676)
    if (value == 2308452 )
    {
      Serial.println("alarma desactivada");
      //digitalWrite(SIRENA, LOW);
      digitalWrite(LEDGSM, LOW);

      desactivar ();
    }




    if (value == 11729960)
    {
      Serial.println("Alarma activada");
      digitalWrite(LEDGSM, HIGH);
      activar();

    }

    if (value == 11729956)
    {
      Serial.println("alarma desactivada");
      //digitalWrite(SIRENA, LOW);
      digitalWrite(LEDGSM, LOW);

      desactivar ();
    }



    mySwitch.resetAvailable();

  }

  //----------------------------------rutina de disparo---------------------
  if (estadoz1 && activada)  {
    estadosirena = HIGH;

    tiempoahora = millis ();


    digitalWrite(PGM, HIGH);



  }
  //delay (500);

  if (estadoz1 && !activada) {
    estadosirena = LOW;
    delay (500);

    digitalWrite(PGM, LOW);
  }

  //delay (500);



}
//---------------final void Loop---------------

//---------------Rutinas de activar y desactivar-------------------
void activar () {
  activada = true;
  digitalWrite (SIRENA , HIGH);
  delay(200);
  digitalWrite (SIRENA , LOW);
  delay(200);

}

void desactivar () {
  activada = false;
  estadosirena = LOW;
  digitalWrite (SIRENA , HIGH);
  delay(200);
  digitalWrite (SIRENA , LOW);
  delay(200);
  digitalWrite (SIRENA , HIGH);
  delay(200);
  digitalWrite (SIRENA , LOW);
  delay(200);
}
