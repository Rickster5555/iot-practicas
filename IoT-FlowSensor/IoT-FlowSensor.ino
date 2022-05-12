#include <Ubidots.h>
//#define WIFISSID "Totalplay-799D"
//#define PASSWORD "CONTPAQi83"
#define WIFISSID "Lentisimo"
#define PASSWORD "WiFi1361"
//#define WIFISSID "IoT"
//#define PASSWORD "1t3s0IoT18"
#define TOKEN "BBFF-zYZGrqqAov1v3srMpvQ3OWr6Z7d0XF"

Ubidots client(TOKEN);

float m1 = 1.0;
float m2 = 3.0;
float m3 = 5.0;

const int sensorPin = 13;
#define LED1  15
#define LED2  2
#define LED3  4

const int measureInterval = 2500;
volatile int pulseConter;
float vol = 0;
long dt = 0; //variación de tiempo por cada bucle
long t0 = 0; //millis() del bucle anterior

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// ARD-370 - Modelo de nuestro caudalimetro
const float factorK = 7.5; //factor de la frecuencia (Hz( del pulso o flujo del caudal
//segun el fabricante, en este caso Steren


//funcion que cuenta y suma los pulsos
void ISRCountPulse()
{
  pulseConter++;
}

//funcion para obtener la cantidad de pulsos en el intervalo establecido al principio
float GetFrequency()
{
  pulseConter = 0;

  interrupts();
  delay(measureInterval);
  noInterrupts();

  return (float)pulseConter * 1000 / measureInterval;
}

void setup()
{
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
  Serial.println ("Envie 'r' para restablecer el volumen a 0 Litros");
  t0 = millis();
  pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED2, LOW);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED3, LOW);

  client.wifiConnect(WIFISSID, PASSWORD);
  client.setDebug(true);
  Serial.println();
  Serial.print("Wait for WiFi...");
  Serial.println();
  
  

  
}

void loop()
{
  if (Serial.available()) {
    if (Serial.read() == 'r') {
      vol = 0; //restablecemos el volumen si recibimos 'r'
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
    }
  }

  // obtener frecuencia en Hz
  float frequency = GetFrequency();

  // calcular caudal L/min
  float flow = frequency / factorK;

  // obtener volumen en Litros
  dt = millis() - t0; //calculamos la variación de tiempo
  t0 = millis();
  vol = vol + (flow / 60) * (dt / 1000); // volumen(L)=caudal(L/s)*tiempo(s)

  if (vol >= m1) {
    digitalWrite(LED1, HIGH);
  }
  if (vol >= m2) {
    digitalWrite(LED2, HIGH);
  }
  if (vol >= m3) {
    digitalWrite(LED3, HIGH);
  }

  Serial.print("Frecuencia: ");
  Serial.print(frequency, 0);
  Serial.print(" (Hz)\tCaudal: ");
  Serial.print(flow, 2);
  Serial.println(" (L/min)");
  Serial.print("Volumen: ");
  Serial.print(vol, 2);
  Serial.println(" Litros");

 
  client.add("flujo", flow);
  client.add("volumen", vol);
  
  //m1 = m1 + 1;
  //client.add("l1", m1);
  //client.add("l2", m2);
  //client.add("l3", m3);
  
  bool bufferSent = false;
  bufferSent = client.send();

  if (bufferSent) {
    Serial.println("Datos publicados");
    }
  delay(2000); // 15 segundos en milisegundos entre publicaciones en client
}
